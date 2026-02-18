using System.Numerics;
using Raylib_cs;

public class Program
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    static Ball ball = new Ball(_position:new Vector2(0,0), _radius:20, 
                                _mass:1, _springConstant:200, _dampingFactor:25, _colour:Color.Red);
    static float offsetX = screenWidth / 2;
    static float offsetY = screenHeight / 2;
    static List<Wall> walls = new List<Wall>();
    public static float forceCutThreshold = 35000;
    public static float cutWaitTime = 0.1f;
    public static float cutTimer = 0f;
    static void Start()
    {
        Raylib.InitWindow(screenWidth, screenHeight, "Haptic Simulation");
        Raylib.SetTargetFPS(60);
        walls.Add(new Wall(new Vector2(0, 20)));
        for(int i = 0; i < 10; i++)
        {
            walls[0].vertices.Add(new Vertex(new Vector2(walls[0].position.X + (i-5) * 50, walls[0].position.Y)));
            if(i == 0 || i == 9) walls[0].vertices[i].freeze = true;
        }
    }

    public static void Main()
    {
        Start();

        while (!Raylib.WindowShouldClose())
        {
            if(cutTimer > 0) cutTimer -= Raylib.GetFrameTime();
            if(cutTimer < 0) cutTimer = 0;

            List<Wall> nextWalls = new List<Wall>();
            Vector2 mousePosition = Raylib.GetMousePosition() - new Vector2(offsetX, offsetY);
            (Vector2 normalForce, nextWalls) = Physics.ApplyBallPositionToMouse(ball, mousePosition, walls);
            normalForce /= 1000;

            foreach(Wall wall in walls) wall.UpdateVertices();

            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.LightGray);
            Raylib.DrawCircle((int)(ball.position.X + offsetX), (int)(ball.position.Y+offsetY), ball.radius, ball.colour);
            
            foreach(Wall wall in walls)
            {
                for(int i = 0; i < wall.vertices.Count(); i++)
                {
                    int posX = (int)(wall.vertices[i].position.X + offsetX + wall.position.X);
                    int posY = (int)(wall.vertices[i].position.Y + offsetY + wall.position.Y);
                    Raylib.DrawCircle(posX, posY, 5f, Color.DarkGray);

                    if(i < wall.vertices.Count() - 1)
                    {
                        int nextPosX = (int)(wall.vertices[i+1].position.X + offsetX + wall.position.X);
                        int nextPosY = (int)(wall.vertices[i+1].position.Y + offsetY + wall.position.Y);

                        Raylib.DrawLine(posX, posY, nextPosX, nextPosY, Color.DarkGray);
                    }
                }
            }
            
            Raylib.DrawLine((int)(ball.position.X + offsetX), (int)(ball.position.Y + offsetY),
                            (int)(ball.position.X + offsetX + normalForce.X), (int)(ball.position.Y + offsetY + normalForce.Y),
                            Color.White);

            Raylib.EndDrawing();   

            walls = nextWalls.ToList();
        }

        Raylib.CloseWindow();
    }
}

public class Ball
{
    public Vector2 position;
    public Vector2 velocity;
    public float radius;
    public float mass;
    public float springConstant;
    public float dampingFactor;
    public Color colour;

    public Ball(Vector2 _position, float _radius,  float _mass, float _springConstant, float _dampingFactor, Color _colour)
    {
        position = _position;
        radius = _radius;
        mass = _mass;
        colour = _colour;
        springConstant = _springConstant;
        dampingFactor = _dampingFactor;
    }
}

public class Vertex
{
    public Vector2 position;
    public Vector2 restPosition {get; private set;}
    public Vector2 velocity = Vector2.Zero;
    public bool isFreeEnd = false;
    public bool freeze = false;
    public float bondLength = 25;

    public Vertex(Vector2 _position)
    {
        position = _position;
        restPosition = _position;
    }
}

public class Wall
{
    public Vector2 position;
    public List<Vertex> vertices = new List<Vertex>();
    public float springConstant = 100;
    public float dampingCoefficient = 0.1f;

    public Wall(Vector2 _position)
    {
        position = _position;
    }

    public void UpdateVertices()
    {
        for(int i = 0; i < vertices.Count(); i++)
        {
            Vertex thisVert = vertices[i];
            // Calculate restoring force
            if(thisVert.freeze) continue;

            Vector2 force = Vector2.Zero;
            if(i == 0)
            {
                if(vertices.Count <= 1) continue;
                Vector2 dir = Vector2.Normalize(vertices[i+1].position - thisVert.position);
                force = springConstant * (vertices[i+1].position - thisVert.position - (thisVert.bondLength + vertices[i+1].bondLength)  * dir);
            }
            else if(i == vertices.Count - 1)
            {
                if(vertices.Count <= 1) continue;
                Vector2 dir = Vector2.Normalize(vertices[i-1].position - thisVert.position);
                force = springConstant * (vertices[i-1].position - thisVert.position - (thisVert.bondLength + vertices[i-1].bondLength)  * dir);
            }
            else
            {
                force = springConstant * (vertices[i-1].position + vertices[i+1].position - 2*thisVert.position); 
            }
            force += new Vector2(0, 1) * 20;
            thisVert.velocity += force * Raylib.GetFrameTime() - thisVert.velocity * dampingCoefficient;
            thisVert.position += thisVert.velocity * Raylib.GetFrameTime();
        }
    }
}