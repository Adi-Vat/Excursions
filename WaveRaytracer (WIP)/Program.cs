using System.Numerics;
using Raylib_cs;

static class WaveRaytracer
{
    const int width = 800;
    const int height = 600;
    static List<Source> sources = new List<Source>();
    const int numberRaysPerSource = 8;
    static List<Collider> colliders = new List<Collider>();

    static void Start()
    {
        sources.Add(new Source(new Vector2(0, 0), 100));
    }

    public static void Main()
    {
        Start();
        Raylib.InitWindow(width, height, "Wave Raytracer");

        while (!Raylib.WindowShouldClose())
        {
            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.White);
            ComputeSources();
            Raylib.EndDrawing();
        }
    }

    static void ComputeSources()
    {
        foreach(Source source in sources)
        {
            Vector2 screenPosition = source.position;
            screenPosition.X += width / 2;
            screenPosition.Y += height/ 2;
            Raylib.DrawCircle((int)screenPosition.X, (int)screenPosition.Y, 5f, Color.Red);
            
            for(int i = 0; i < numberRaysPerSource; i++)
            {
                double angle = 2*Math.PI/numberRaysPerSource * i;
                double xDir = Math.Cos(angle);
                double yDir = Math.Sin(angle);
                Vector2 rayDir = Vector2.Normalize(new Vector2((float)xDir, (float)yDir));
                Vector2 endScreenPos = screenPosition + rayDir * 500;
                Raylib.DrawLine((int)screenPosition.X, (int)screenPosition.Y, (int)endScreenPos.X, (int)endScreenPos.Y, Color.Red);
            }
        }
    }

    static float distanceToNearestCollider(Vector2 point)
    {
        float nearestDistance = -1;

        foreach(Collider col in colliders)
        {
            if(col.points.Count < 2) continue;
            // Get edge/line
            for(int i = 0; i < col.points.Count - 2; i++)
            {
                Vector2 p1 = col.points[i];
                Vector2 p2 = col.points[i+1];

                float A = p1.Y - p2.Y;
                float B = p2.X - p1.X;
                float C = p1.X*p2.Y - p2.X*p1.Y;

                float d = (float)(Math.Abs(A * point.X + B*point.Y + C)/Math.Sqrt((A*A + B*B)));
                if(d < nearestDistance || nearestDistance < 0) nearestDistance = d;
            }
        }

        return nearestDistance;
    }

    static void ComputeRay(Vector2 startPosition, Vector2 direction, float length)
    {
        
    }

    static void DrawScreen()
    {
        
    }
}

class Source
{
    public Vector2 position;
    public float frequency;

    public Source(Vector2 _position, float _frequency)
    {
        position = _position;
        frequency = _frequency;
    }
}

class Collider
{
    public List<Vector2> points = new List<Vector2>();
}