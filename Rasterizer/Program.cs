using System.Numerics;
using Raylib_cs;

class Program
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    const int distanceToCamera = 50;
    const float moveSpeed = 0.1f;
    
    static Vector3 cameraPosition = new Vector3(0,0,-2);
    static Vector3 sunDirection = new Vector3(0.5f, -0.5f, 0);

    static void Main()
    {
        Mesh[] meshes =
        {
            new Mesh(
                new Vector3(0, 0, 5),
                [
                    new Vector3(-1, -1, -1),
                    new Vector3(1, -1, -1),
                    new Vector3(1, 1, -1),
                    new Vector3(-1, 1, -1),
                    new Vector3(1, -1, 1),
                    new Vector3(1, 1, 1),
                    new Vector3(-1, -1, 1),
                    new Vector3(-1, 1, 1)
                ],
                [
                    [0, 1, 2],
                    [0, 2, 3],
                    [1, 4, 5],
                    [1, 5, 2],
                    [4, 6, 7],
                    [4, 7, 5],
                    [3, 2, 5],
                    [3, 5, 7],
                    [6, 4, 1],
                    [6, 1, 0],
                    [0, 3, 7],
                    [0, 7, 6]
                ]
            ),
        };

        Raylib.SetTraceLogLevel(TraceLogLevel.Warning);
        Raylib.InitWindow(screenWidth, screenHeight, "Rasterizer");
        Raylib.SetTargetFPS(60);


        Image img = Raylib.GenImageColor(screenWidth, screenHeight, Raylib_cs.Color.Black);
        Texture2D texture = Raylib.LoadTextureFromImage(img);
        Raylib.UnloadImage(img);

        float[] depthBuffer = new float[screenWidth * screenHeight];

        while (!Raylib.WindowShouldClose())
        {   
            if(Raylib.IsKeyDown(KeyboardKey.Up))cameraPosition.Z += moveSpeed;
            if(Raylib.IsKeyDown(KeyboardKey.Down))cameraPosition.Z -= moveSpeed;
            if(Raylib.IsKeyDown(KeyboardKey.Left))cameraPosition.X -= moveSpeed;
            if(Raylib.IsKeyDown(KeyboardKey.Right))cameraPosition.X += moveSpeed;
            if(Raylib.IsKeyDown(KeyboardKey.PageUp))cameraPosition.Y += moveSpeed;
            if(Raylib.IsKeyDown(KeyboardKey.PageDown))cameraPosition.Y -= moveSpeed;
            
            Mesh cube = meshes[0];
            float rotation = 0.01f;
            for(int i = 0; i < cube.vertices.Length; i++){
                Vector3 vertex = cube.vertices[i];

                float rotatedX = (float)(vertex.X * Math.Cos(rotation) - vertex.Z * Math.Sin(rotation));
                float rotatedZ = (float)(vertex.X * Math.Sin(rotation) + vertex.Z * Math.Cos(rotation));
                //float rotatedY = (float)(vertex.Y * Math.Cos(rotation) - vertex.Z * Math.Sin(rotation));
                float rotatedY = vertex.Y;
                meshes[0].vertices[i] = new Vector3(rotatedX, rotatedY, rotatedZ);
                // Now project (rotatedX, vertex.Y, rotatedZ) to screen
            }

            for(int i = 0; i < depthBuffer.Length; i++)
            {
                depthBuffer[i] = float.MaxValue;
            }

            Raylib.BeginDrawing();
            Raylib.ClearBackground(Raylib_cs.Color.Black);

            unsafe
            {
                Image frameImg = Raylib.LoadImageFromTexture(texture);
                Raylib_cs.Color* pixels = (Raylib_cs.Color*)frameImg.Data;

                for (int i = 0; i < screenWidth * screenHeight; i++) {
                    pixels[i] = Raylib_cs.Color.Black;
                }

                foreach(Mesh mesh in meshes)
                {
                    Vector3[] verts = mesh.vertices;
                    Vector3[] newVerts = new Vector3[verts.Length];
                    Vector3 meshOrigin = mesh.origin;
                    int[][] tris = mesh.triangles;

                    for(int i = 0; i < verts.Length; i++)
                    {
                        newVerts[i] = verts[i] + meshOrigin;
                    }

                    // Draw triangles
                    // loop through each triangle
                    for(int i = 0; i < tris.Length; i++)
                    {
                        int[] triangle = tris[i];
                        Vector3[] triangleScreenPoints = new Vector3[3];

                        // transform world space into screen space
                        triangleScreenPoints[0] = transformPointWorldToScreen(newVerts[triangle[0]]);
                        triangleScreenPoints[1] = transformPointWorldToScreen(newVerts[triangle[1]]);
                        triangleScreenPoints[2] = transformPointWorldToScreen(newVerts[triangle[2]]);
                        
                        Vector3 edge1 = newVerts[triangle[1]] - newVerts[triangle[0]];
                        Vector3 edge2 = newVerts[triangle[2]] - newVerts[triangle[0]];

                        Vector3 normal = Vector3.Cross(edge1, edge2);
                        normal = Vector3.Normalize(normal);
                        float dotProduct = Vector3.Dot(normal, sunDirection);

                        dotProduct = (dotProduct + 1) / 2;
                        int newR = (int)(255 * dotProduct);

                        Raylib_cs.Color color = new Raylib_cs.Color(newR, 0, 0);

                        // If a face is facing away from the camera, ignore it.
                        Vector3 dirToCamera = newVerts[triangle[0]] - cameraPosition;
                        if(Vector3.Dot(normal, dirToCamera) < 0) continue;

                        // Draw edge between two vertices
                        float maxY = -screenHeight;
                        float minY = screenHeight;
                        float maxX = -screenWidth;
                        float minX = screenWidth;

                        for(int k = 0; k < 3; k++)
                        {
                            int nextK = (k==2) ? 0 : k+1;
                            Vector3 point = triangleScreenPoints[k];
                            Vector3 nextPoint = triangleScreenPoints[nextK];

                            //Raylib.DrawLine((int)point.X, (int)point.Y, (int)nextPoint.X, (int)nextPoint.Y, Raylib_cs.Color.Green);
                            if(point.X > maxX) maxX = point.X;
                            if(point.X < minX) minX = point.X;
                            if(point.Y > maxY) maxY = point.Y;
                            if(point.Y < minY) minY = point.Y;
                        }

                        for(int x = (int)minX; x < (int)maxX; x++)
                        {
                            for(int y = (int)minY; y < (int)maxY; y++)
                            {  
                                if (x < 0 || x >= screenWidth || y < 0 || y >= screenHeight) 
                                    continue;

                                float u,v,w;
                                (u,v,w) = calculateBarymetricValue(triangleScreenPoints[0].AsVector2(),triangleScreenPoints[1].AsVector2(),triangleScreenPoints[2].AsVector2(), new Vector2(x,y));
                                if(u+v+w <= 1)
                                {
                                    float z0 = triangleScreenPoints[2].Z;
                                    float z1 = triangleScreenPoints[1].Z;
                                    float z2 = triangleScreenPoints[0].Z;
                                    float zDistance = u * z0 + v * z1 + w * z2;
                                    if(zDistance < depthBuffer[y * screenWidth + x])
                                    {
                                        depthBuffer[y * screenWidth + x] = zDistance;
                                        pixels[y * screenWidth + x] = color;
                                    }
                                }
                            }
                        }
                    }
                }

                Raylib.UpdateTexture(texture, pixels);
                Raylib.UnloadImage(frameImg);
            }

            Raylib.DrawTexture(texture, 0, 0, Raylib_cs.Color.White);

            Raylib.EndDrawing();
        }
        Raylib.UnloadTexture(texture);
        Raylib.CloseWindow();
    }

    static Vector3 transformPointWorldToScreen(Vector3 worldPoint)
    {

        float dx = worldPoint.X - cameraPosition.X;
        float dy = worldPoint.Y - cameraPosition.Y;
        float dz = worldPoint.Z - cameraPosition.Z;

        float focalLength = 500;

        float screenXFloat = (dx * focalLength) / dz;
        float screenYFloat = (dy * focalLength) / dz;

        int screenX = (int)screenXFloat + screenWidth/2;
        int screenY = -(int)screenYFloat + screenHeight/2;
        
        Vector3 screenPoint =  new Vector3(screenX, screenY, dz);
        return screenPoint;
    }

    static (float,float,float) calculateBarymetricValue(Vector2 a, Vector2 b, Vector2 c, Vector2 p)
    {
        float areaABC = triangleArea(a,b,c);
        float areaBPC = triangleArea(b,p,c);
        float areaAPC = triangleArea(a,p,c);
        float areaAPB = triangleArea(a,p,b);

        float u = areaAPB/areaABC;
        float v = areaAPC/areaABC;
        float w = areaBPC/areaABC;

        return (u, v, w);
    }

    static float triangleArea(Vector2 point0, Vector2 point1, Vector2 point2)
    {
        Vector2 ca = point2-point0;
        Vector2 ba = point1-point0;
        float area = 0.5f * Math.Abs(Vector2.Cross(ca, ba));
        return area;
    }
}

class Mesh
{
    public Vector3 origin;
    public Vector3[] vertices {get; private set;}
    // which 3 vertices (referenced by index) make a triangle together!
    public int[][] triangles {get; private set;}

    public Mesh(Vector3 _origin, Vector3[] _vertices, int[][] _triangles)
    {
        origin = _origin;
        vertices = _vertices;
        triangles = _triangles;
    }
}