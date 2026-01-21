using System.Numerics;
using System.Reflection.Metadata.Ecma335;
using Raylib_cs;
using System.Runtime.CompilerServices;

class Program
{
    const int screenWidth = 1200;
    const int screenHeight = 800;
    const float ROOT_3 = 1.7320508f;
    static Camera2D camera;
    static List<MeasurementPoint> pointCloud = new List<MeasurementPoint>();
    const int pointRadius = 5;
    static List<Circumcircle> circumcircles = new List<Circumcircle>();
    static List<Triangle> triangles = new List<Triangle>();

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
        Vector2 mouseWorldPos = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
        MeasurementPoint newPoint = new MeasurementPoint(mouseWorldPos, 1f);
        pointCloud.Add(newPoint);
    }

    static void RemoveMeasurementPoint()
    {
        Vector2 mouseWorldPos = Raylib.GetScreenToWorld2D(Raylib.GetMousePosition(), camera);
        MeasurementPoint? pointToRemove = null;

        foreach(MeasurementPoint point in pointCloud)
        {
            // Check if the mouse hovers over any point
            float distToMouse = Vector2.Distance(mouseWorldPos, point.position);
            if(distToMouse <= pointRadius)
            {
                pointToRemove = point;
                break;
            }
        }
        
        if(pointToRemove != null) pointCloud.Remove(pointToRemove);
    }

    static void ComputeBoyerWatson()
    {
        List<Triangle> triangulation = new List<Triangle>();
        Triangle superTriangle = GetSuperTriangle();
        triangulation.Add(superTriangle);

        foreach(MeasurementPoint vertex in pointCloud)
        {
            List<Triangle> badTriangles = new List<Triangle>();
            Dictionary<Edge, int> edgeDictionary = new Dictionary<Edge, int>();

            foreach(Triangle tri in triangulation)
            {
                // Check if the vertex is inside the circumcircle of this triangle
                (bool validTriangle, Circumcircle circumcircle) = 
                    getCircumcircle(tri.verts[0].position, tri.verts[1].position, tri.verts[2].position);
                if(!validTriangle) continue;
                // Distance to circumcircle
                float distSq = Vector2.DistanceSquared(vertex.position, circumcircle.centre);
                if(distSq <= circumcircle.radius * circumcircle.radius) badTriangles.Add(tri); 
            }

            List<Edge> polygon = new List<Edge>();
            foreach(Triangle badTri in badTriangles)
            {
                MeasurementPoint[] badVerts = badTri.verts;
                edgeDictionary = CountEdge(edgeDictionary, badVerts[0], badVerts[1]);     
                edgeDictionary = CountEdge(edgeDictionary, badVerts[1], badVerts[2]);     
                edgeDictionary = CountEdge(edgeDictionary, badVerts[2], badVerts[0]);
            }

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
        foreach(MeasurementPoint point in pointCloud)
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
        (Vector2 p1, Vector2 p2, Vector2 p3) = getEquilateralTriangleFromIncircle(largestCircleCentre, largestCircleRadius);
        
        MeasurementPoint point1 = new MeasurementPoint(p1, 0);
        MeasurementPoint point2 = new MeasurementPoint(p2, 0);
        MeasurementPoint point3 = new MeasurementPoint(p3, 0);

        Triangle superTriangle = new Triangle([point1, point2, point3]);
        return superTriangle;
    }

    static void DrawHeatmap()
    {   
        
        foreach(MeasurementPoint point in pointCloud)
        {
            Raylib.DrawCircle((int)point.position.X, (int)point.position.Y, pointRadius, Color.Red);
        }
        
        foreach(Triangle tri in triangles)
        {
            Raylib.DrawTriangleLines(tri.verts[0].position, tri.verts[1].position, tri.verts[2].position, Color.Red);
        }
    }

    public static void Main()
    {
        camera.Zoom = 1.0f;
        Raylib.SetConfigFlags(ConfigFlags.ResizableWindow);    
        Raylib.InitWindow(screenWidth, screenHeight, "RF Heatmap");
        Image img = Raylib.LoadImage("C:/Users/Adi/Documents/Excursions/RFHeatmap/floormaps/868_Floor2.png");
        Texture2D texture = Raylib.LoadTextureFromImage(img);

        Raylib.SetTargetFPS(60);

        while (!Raylib.WindowShouldClose())
        {
            PanImage();
            CamZoom();

            if(Raylib.IsMouseButtonPressed(MouseButton.Left)) AddMeasurementPoint();
            if(Raylib.IsMouseButtonPressed(MouseButton.Right)) RemoveMeasurementPoint();
            if(Raylib.IsKeyPressed(KeyboardKey.Space)) ComputeBoyerWatson();

            Raylib.BeginDrawing();
            Raylib.BeginMode2D(camera);
            
            Raylib.ClearBackground(Color.Black);
            Raylib.DrawTexture(texture, 0, 0, Color.White);
            DrawHeatmap();
            Raylib.EndMode2D();
            Raylib.EndDrawing();
        }

        Raylib.UnloadTexture(texture);
        Raylib.CloseWindow();
    }

    static (bool validTriangle, Circumcircle returnCircle) getCircumcircle(Vector2 p1, Vector2 p2, Vector2 p3)
    {
        float radius = 0;
        Vector2 centre = Vector2.Zero;

        float grad1 = (p1.Y - p2.Y)/(p1.X - p2.X);
        float grad2 = (p1.Y - p3.Y)/(p1.X - p3.X);

        if(grad1 == grad2) return(false, new Circumcircle(Vector2.Zero, 0));

        // Caclulate perp. bisector of p1,p2
        Vector2 midpoint12 = new Vector2(0.5f*(p1.X + p2.X), 0.5f*(p1.Y + p2.Y));
        float gradPerpBisector12 = -1/grad1;
        // y = mx + c
        // 0 = mx + c - y
        // -c = mx - y
        // c = -(mx - y)
        float c12 = -(gradPerpBisector12 * midpoint12.X - midpoint12.Y);

        // Calculate perp. bisector p1,p3
        Vector2 midpoint13 = new Vector2(0.5f*(p1.X + p3.X), 0.5f*(p1.Y + p3.Y));
        float gradPerpBisector13 = -1/grad2;
        float c13 = -(gradPerpBisector13 * midpoint13.X - midpoint13.Y);

        // Find intersection of two perp. bisectors
        // y = m1x + c1
        // y = m2x + c2
        // m1x + c1 = m2x + c2
        // m1x = m2x + c2 - c1
        // x (m1-m2) = c2-c1
        // x = (c2-c1)/(m1-m2)
        float xIntersect = (c13 - c12)/(gradPerpBisector12 - gradPerpBisector13);
        float yIntersect = gradPerpBisector12 * xIntersect + c12;

        centre = new Vector2(xIntersect, yIntersect);

        radius = Vector2.Distance(p1, centre);
        
        return (true, new Circumcircle(centre, radius));
    }

    static (Vector2 p1, Vector2 p2, Vector2 p3) getEquilateralTriangleFromIncircle(Vector2 centre, float radius)
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
}

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