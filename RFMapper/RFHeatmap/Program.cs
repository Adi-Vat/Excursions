using System.Numerics;
using Raylib_cs;
using System.Runtime.CompilerServices;
using System.IO.Ports;

class Program
{
    // constants
    const int screenWidth = 1200;
    const int screenHeight = 800;
    const float ROOT_3 = 1.7320508f;
    const int pointRadius = 5;
    // screen stuff
    static Camera2D camera;
    static Texture2D screenTexture;
    // arduino stuff
    static SerialPort arduino = new SerialPort();
    static bool usingArduino = false;
    // triangulation stuff
    static List<MeasurementPoint>[] pointCloud = new List<MeasurementPoint>[4];
    const int minValue = -80;
    const int maxValue = -25;
    static MeasurementPoint? currentlySelectedPoint;
    static List<Triangle> triangles = new List<Triangle>();
    static float[] frequencies = {869.70f, 867.40f, 868.65f, 866.70f};  
    static int layer = 0;
    // point editing stuff
    static bool editingPoint = false;
    static string editingPointValueStr = "";
    static bool showPointStrength = false;
    
    #region Screen
    static void PanImage()
    {
        if (Raylib.IsMouseButtonDown(MouseButton.Middle))
        {
            Vector2 mouseDelta = Raylib.GetMouseDelta();
            camera.Offset += mouseDelta;
        }

        if (Raylib.IsKeyPressed(KeyboardKey.R))
        {
            camera.Target = Vector2.Zero;
            camera.Offset = Vector2.Zero;
        }
    }
    static void CamZoom()
    {
        float wheel = Raylib.GetMouseWheelMove();
        Vector2 mouseWorldPos = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
        camera.Offset = Raylib.GetMousePosition();
        camera.Target = mouseWorldPos;
        float scale = 0.2f*wheel;
        camera.Zoom = Math.Clamp(MathF.Exp(MathF.Log(camera.Zoom) + scale), 0.125f, 64.0f);
    }
    static void AddMeasurementPoint()
    {
        editingPointValueStr = "";
        MeasurementPoint? nearestPoint = null;
        nearestPoint = getClosestPointToMouse();
        if(nearestPoint != null)
        {
            editingPointValueStr = nearestPoint.strength.ToString();
            editingPoint = true;
            currentlySelectedPoint = nearestPoint;
            return;
        }

        Vector2 mouseWorldPos = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
        MeasurementPoint newPoint = new MeasurementPoint(mouseWorldPos, 1f);
        newPoint.strength = 0;
        editingPoint = true;
        currentlySelectedPoint = newPoint;
        pointCloud[layer].Add(newPoint);
    }   

    static void RemoveMeasurementPoint()
    {
        MeasurementPoint? pointToRemove = getClosestPointToMouse();
        
        if(pointToRemove != null) pointCloud[layer].Remove(pointToRemove);
    }

    static void MoveMeasurementPoint()
    {
        if(currentlySelectedPoint == null) return;

        editingPoint = false;
        currentlySelectedPoint.position = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
    }

    static MeasurementPoint? getClosestPointToMouse()
    {
        Vector2 mouseWorldPos = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
        MeasurementPoint? selectedPoint = null;

        foreach(MeasurementPoint point in pointCloud[layer])
        {
            // Check if the mouse hovers over any point
            float distToMouse = Vector2.Distance(mouseWorldPos, point.position);
            if(distToMouse <= pointRadius)
            {
                selectedPoint = point;
                break;
            }
        }

        return selectedPoint;
    }
    
    static void DrawHeatmap()
    {
        Rlgl.Begin(4);
        foreach(Triangle triangle in triangles)
        {
            // One triangle, three points
            for(int i = 0; i < 3; i++)
            {
                // Define color for next vertex
                float lerpValue = (triangle.verts[i].strength - minValue) / (maxValue - minValue);
                
                if(lerpValue < 0.2f) Rlgl.Color4ub(0, 0, 255, 125);
                else if(lerpValue < 0.4f) Rlgl.Color4ub(0, 255, 0, 125);
                else if(lerpValue < 0.8f) Rlgl.Color4ub(255, 255, 0, 125);
                else Rlgl.Color4ub(255, 0, 0, 125);

                // Define vertex
                Rlgl.Vertex2f(triangle.verts[i].position.X, triangle.verts[i].position.Y);
            }
        }
        Rlgl.End();
        foreach(MeasurementPoint measurementPoint in pointCloud[layer])
        {
            bool drawTextLocal = false;
            Raylib.DrawCircle((int)measurementPoint.position.X, (int)measurementPoint.position.Y, 2f, Color.Black);
            if(measurementPoint == currentlySelectedPoint)
            {
                Raylib.DrawCircle((int)measurementPoint.position.X, (int)measurementPoint.position.Y, 1.5f, Color.White);
                drawTextLocal = true;
            }
            else if(showPointStrength) drawTextLocal = true;

            if(drawTextLocal)
                 Raylib.DrawText(measurementPoint.strength.ToString(), (int)measurementPoint.position.X + 5, (int)measurementPoint.position.Y + 5, 18, Color.Black);
        }
    }
    static void DrawScreen(Texture2D texture)
    {
        Raylib.BeginDrawing();
        Raylib.BeginMode2D(camera);
        Raylib.ClearBackground(Color.Black);
        Raylib.DrawTexture(texture, 0, 0, Color.White);
        DrawHeatmap();
        Raylib.EndMode2D();
        Raylib.DrawText("Layer " + (layer + 1) + "  " + frequencies[layer] + " MHz", 10, 10, 40, Color.Black);
        Raylib.EndDrawing();
    }

    static void ChangeLayer(int newLayer)
    {
        layer = newLayer;
        ComputeBowyerWatson();
        
        if(usingArduino) arduino.WriteLine("L"+newLayer);
    }
    #endregion

    #region Triangulation
    /* Performs the Bowyer-Watson Delaunay triangle algorithm to generate best-case triangles
    with the most even internal angles
    https://www.gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation/ */
    static void ComputeBowyerWatson()
    {
        List<Triangle> triangulation = new List<Triangle>();
        Triangle superTriangle = GetSuperTriangle();
        triangulation.Add(superTriangle);

        foreach(MeasurementPoint vertex in pointCloud[layer])
        {
            List<Triangle> badTriangles = new List<Triangle>();
            Dictionary<Edge, int> edgeDictionary = new Dictionary<Edge, int>();

            foreach(Triangle tri in triangulation)
            {
                // Check if the vertex is inside the circumcircle of this triangle
                (bool validTriangle, Circumcircle circumcircle) = 
                    GetCircumcircle(tri.verts[0].position, tri.verts[1].position, tri.verts[2].position);
                if(!validTriangle) continue;
                // Distance to circumcircle
                float distSq = Vector2.DistanceSquared(vertex.position, circumcircle.centre);
                if(distSq <= circumcircle.radius * circumcircle.radius) badTriangles.Add(tri); 
            }

            // Get a list of outside edges of the bad triangles
            List<Edge> polygon = new List<Edge>();
            foreach(Triangle badTri in badTriangles)
            {
                MeasurementPoint[] badVerts = badTri.verts;
                edgeDictionary = CountEdge(edgeDictionary, badVerts[0], badVerts[1]);     
                edgeDictionary = CountEdge(edgeDictionary, badVerts[1], badVerts[2]);     
                edgeDictionary = CountEdge(edgeDictionary, badVerts[2], badVerts[0]);
            }

            // Create the polygonal hole of bad triangles
            foreach(Edge badEdge in edgeDictionary.Keys)
            {
                if(edgeDictionary[badEdge] == 1)
                {
                    polygon.Add(badEdge);
                }
            }

            foreach(Triangle badTriangle in badTriangles)
            {
                triangulation.Remove(badTriangle);
            }

            foreach(Edge badEdge in polygon)
            {
                Triangle newTriangle = new Triangle([vertex, badEdge.A, badEdge.B]);
                triangulation.Add(newTriangle);
            }
        }
        
        // Remove all triangles connected to the inital super triangle
        for(int k = triangulation.Count - 1; k >= 0; k--)
        {
            bool removeTriangle = false;
            for(int i = 0; i < 3; i++)
            {       
                if (triangulation[k].verts.Contains(superTriangle.verts[i]))
                {
                    removeTriangle = true;
                }
            }
            if (removeTriangle)
            {
                triangulation.RemoveAt(k);
            }
        }

        triangles = triangulation;
    }

    // Counts how many times an edge is used by triangles
    static Dictionary<Edge, int> CountEdge(Dictionary<Edge, int> dict, MeasurementPoint a, MeasurementPoint b)
    {
        Edge edge = new Edge(a,b);

        if(dict.TryGetValue(edge, out int count))
        {
            dict[edge] = count + 1;
        }
        else
        {
            dict[edge] = 1;
        }
        return dict;
    }

    static Triangle GetSuperTriangle()
    {
        Vector2 topLeftPoint = Vector2.NaN;
        Vector2 bottomRightPoint = Vector2.NaN;
        foreach(MeasurementPoint point in pointCloud[layer])
        {
            if(point.position.X < topLeftPoint.X || float.IsNaN(topLeftPoint.X))
                topLeftPoint.X = point.position.X;
        
            if(point.position.Y > topLeftPoint.Y || float.IsNaN(topLeftPoint.Y))
                topLeftPoint.Y = point.position.Y;

            if(point.position.X > bottomRightPoint.X || float.IsNaN(bottomRightPoint.X))
                bottomRightPoint.X = point.position.X;
            
            if(point.position.Y < bottomRightPoint.Y || float.IsNaN(bottomRightPoint.Y))
                bottomRightPoint.Y = point.position.Y;
        }

        Vector2 largestCircleCentre = new Vector2(0.5f*(topLeftPoint.X + bottomRightPoint.X), 0.5f*(topLeftPoint.Y + bottomRightPoint.Y));
        float largestCircleRadius = Vector2.Distance(largestCircleCentre, topLeftPoint);
        (Vector2 p1, Vector2 p2, Vector2 p3) = GetEquilateralTriangleFromIncircle(largestCircleCentre, largestCircleRadius);
        
        MeasurementPoint point1 = new MeasurementPoint(p1, 0);
        MeasurementPoint point2 = new MeasurementPoint(p2, 0);
        MeasurementPoint point3 = new MeasurementPoint(p3, 0);

        Triangle superTriangle = new Triangle([point1, point2, point3]);
        return superTriangle;
    }

    static (bool valid, Circumcircle circle) GetCircumcircle(Vector2 a, Vector2 b, Vector2 c)
    {
        float d = 2 * (a.X*(b.Y-c.Y) + b.X*(c.Y-a.Y) + c.X*(a.Y-b.Y));
        if (MathF.Abs(d) < 1e-6f)
            return (false, new Circumcircle(Vector2.Zero, 0));

        float aSq = a.LengthSquared();
        float bSq = b.LengthSquared();
        float cSq = c.LengthSquared();

        float ux = (aSq*(b.Y-c.Y) + bSq*(c.Y-a.Y) + cSq*(a.Y-b.Y)) / d;
        float uy = (aSq*(c.X-b.X) + bSq*(a.X-c.X) + cSq*(b.X-a.X)) / d;

        Vector2 center = new Vector2(ux, uy);
        float radius = Vector2.Distance(center, a);

        return (true, new Circumcircle(center, radius));
    }

    static (Vector2 p1, Vector2 p2, Vector2 p3) GetEquilateralTriangleFromIncircle(Vector2 centre, float radius)
    {
        Vector2 p1 = Vector2.Zero, p2 = Vector2.Zero, p3 = Vector2.Zero;

        p1.X = centre.X;
        p1.Y = centre.Y + 2*radius;

        p2.X = centre.X - radius * ROOT_3;
        p2.Y = centre.Y - radius;
        p3.X = centre.X + radius * ROOT_3;
        p3.Y = centre.Y - radius;

        return (p1, p2, p3);
    }
    #endregion
    
    #region  User Input
    static void GetKeyboardMouseInput()
    {
        if(Raylib.IsMouseButtonPressed(MouseButton.Left)) AddMeasurementPoint();
        if(Raylib.IsMouseButtonPressed(MouseButton.Right)) RemoveMeasurementPoint();
        if(Raylib.IsKeyPressed(KeyboardKey.Space)) ComputeBowyerWatson();
        if(Raylib.IsKeyPressed(KeyboardKey.One)) ChangeLayer(0);
        if(Raylib.IsKeyPressed(KeyboardKey.Two)) ChangeLayer(1);
        if(Raylib.IsKeyPressed(KeyboardKey.Three)) ChangeLayer(2);
        if(Raylib.IsKeyPressed(KeyboardKey.Four)) ChangeLayer(3);
    }

    static void GetTextInput()
    {
        // Get char pressed (unicode character) on the queue
        int key = Raylib.GetCharPressed();
        
        int editingPointValue;  
        if(currentlySelectedPoint == null) return;

        if(Int32.TryParse(editingPointValueStr, out editingPointValue))
        {
            currentlySelectedPoint.strength = editingPointValue;
        }
        else
        {
            currentlySelectedPoint.strength = -0;
        }

        if (Raylib.IsKeyPressed(KeyboardKey.Backspace))
        {
            if(editingPointValueStr.Length == 0) editingPointValueStr = "";
            else editingPointValueStr = editingPointValueStr.Remove(editingPointValueStr.Length - 1);
        }

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125))
            {
                editingPointValueStr += (char)key;
            }
            key = Raylib.GetCharPressed();  // Check next character in the queue
        }

        if (usingArduino && Raylib.IsKeyPressed(KeyboardKey.Tab))
        {
            Console.WriteLine("asking for data");
            arduino.WriteLine(">");   
        }
        
        if (Raylib.IsKeyPressed(KeyboardKey.Enter))
        {
            editingPoint = false;
            currentlySelectedPoint = null;
        }
    }
    #endregion

    #region Arduino
    static void SetupArduino()
    {
        arduino = new SerialPort();
        arduino.PortName = "COM16";
        arduino.BaudRate = 115200;
        arduino.ReadTimeout = 2000;
        arduino.WriteTimeout = 2000;
        arduino.DtrEnable = true; 
        arduino.RtsEnable = true; 
        arduino.DataReceived += new SerialDataReceivedEventHandler(ArduinoDataReceived);
        arduino.Open();
        Thread.Sleep(2000); // Must wait!
        Console.WriteLine("Connected to Arduino!");
    }
    
    private static void ArduinoDataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        // Show all the incoming data in the port's buffer in the output window
        string inData = arduino.ReadExisting();
        Console.WriteLine("data : " + inData);

        float dbmAmount = 0;
        float.TryParse(inData, out dbmAmount);
        Console.WriteLine(dbmAmount);
        if(currentlySelectedPoint != null) currentlySelectedPoint.strength = dbmAmount;
        editingPoint = false;
        currentlySelectedPoint = null;
    }
    #endregion

    static void Start()
    {
        for(int i = 0; i < 4; i++) pointCloud[i] = new List<MeasurementPoint>();

        camera.Zoom = 1.0f;
        Raylib.SetConfigFlags(ConfigFlags.ResizableWindow);    
        Raylib.InitWindow(screenWidth, screenHeight, "RF Heatmap");
        Console.WriteLine(Directory.GetCurrentDirectory());
        string projectRoot = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..", "..");
        Image img = Raylib.LoadImage(Path.Combine(projectRoot, "floormaps\\870.5_Floor2.png"));
        screenTexture = Raylib.LoadTextureFromImage(img);
        Rlgl.DisableBackfaceCulling();
    }

    public static void Main()
    {
        if (usingArduino)SetupArduino();
        
        Start();

        Raylib.SetTargetFPS(60);

        while (!Raylib.WindowShouldClose())
        {
            PanImage();
            CamZoom();

            if(Raylib.IsKeyPressed(KeyboardKey.H)) showPointStrength = !showPointStrength;

            if (editingPoint) GetTextInput();
            else GetKeyboardMouseInput();

            DrawScreen(screenTexture);
        }

        Raylib.UnloadTexture(screenTexture);
        Raylib.CloseWindow();
    }
}

#region Custom Classes
class MeasurementPoint
{
    public Vector2 position;
    public float strength;

    public MeasurementPoint(Vector2 _position, float _strength)
    {
        position = _position;
        strength = _strength;
    }
}

class Edge : IEquatable<Edge>
{
    public MeasurementPoint A;
    public MeasurementPoint B;

    public Edge(MeasurementPoint p1, MeasurementPoint p2)
    {
        // Enforce a canonical order so AB == BA
        if (ReferenceEquals(p1, p2))
            throw new ArgumentException("Edge endpoints must be distinct");

        if (RuntimeHelpers.GetHashCode(p1) <
            RuntimeHelpers.GetHashCode(p2))
        {
            A = p1;
            B = p2;
        }
        else
        {
            A = p2;
            B = p1;
        }
    }

    public bool Equals(Edge other)
        => ReferenceEquals(A, other.A)
        && ReferenceEquals(B, other.B);

    public override bool Equals(object? obj)
        => obj is Edge other && Equals(other);

    public override int GetHashCode()
        => HashCode.Combine(
            RuntimeHelpers.GetHashCode(A),
            RuntimeHelpers.GetHashCode(B));
}

class Triangle
{
    public MeasurementPoint[] verts = new MeasurementPoint[3];

    public Triangle(MeasurementPoint[] _verts)
    {
        verts = _verts;
    }
}

class Circumcircle
{
    public Vector2 centre;
    public float radius;

    public Circumcircle(Vector2 _centre, float _radius)
    {
        centre = _centre;
        radius = _radius;
    }
}
#endregion