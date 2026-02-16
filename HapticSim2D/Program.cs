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

    static void Start()
    {
        Raylib.InitWindow(screenWidth, screenHeight, "Haptic Simulation");
        Raylib.SetTargetFPS(60);
        walls.Add(new Wall(new Vector2(0, 20)));
        walls[0].vertices = [new Vector2(-100, 0), new Vector2(100, 0)];
    }

    public static void Main()
    {
        Start();

        while (!Raylib.WindowShouldClose())
        {
            Vector2 mousePosition = Raylib.GetMousePosition() - new Vector2(offsetX, offsetY);
            Vector2 normalForce = Physics.ApplyBallPositionToMouse(ball, mousePosition, walls);
            normalForce /= 1000;
            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.LightGray);
            Raylib.DrawCircle((int)(ball.position.X + offsetX), (int)(ball.position.Y+offsetY), ball.radius, ball.colour);
            
            foreach(Wall wall in walls)
            {
                for(int i = 0; i < wall.vertices.Count(); i++)
                {
                    int posX = (int)(wall.vertices[i].X + offsetX + wall.position.X);
                    int posY = (int)(wall.vertices[i].Y + offsetY + wall.position.Y);
                    Raylib.DrawCircle(posX, posY, 5f, Color.DarkGray);

                    if(i < wall.vertices.Count() - 1)
                    {
                        int nextPosX = (int)(wall.vertices[i+1].X + offsetX + wall.position.X);
                        int nextPosY = (int)(wall.vertices[i+1].Y + offsetY + wall.position.Y);

                        Raylib.DrawLine(posX, posY, nextPosX, nextPosY, Color.DarkGray);
                    }
                }
            }
            
            Raylib.DrawLine((int)(ball.position.X + offsetX), (int)(ball.position.Y + offsetY),
                            (int)(ball.position.X + offsetX + normalForce.X), (int)(ball.position.Y + offsetY + normalForce.Y),
                            Color.White);

            Raylib.EndDrawing();   
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

public class Wall
{
    public Vector2 position;
    public Vector2[] vertices = new Vector2[0];

    public Wall(Vector2 _position)
    {
        position = _position;
    }
}