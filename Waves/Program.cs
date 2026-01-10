using System.Numerics;
using Raylib_cs;

class Program
{
    const int screenWidth = 1200;
    const int screenHeight = 800;
    const float fieldPerPixel = 0.05f;
    const int fieldWidth = (int)(screenWidth * fieldPerPixel);
    const int fieldHeight = (int)(screenHeight * fieldPerPixel);
    const int C = 500;
    static float[] field = new float[fieldWidth];
    static float[] oldField = new float[fieldWidth];
    static float[] newField = new float[fieldWidth];
    static float timeStep = 0.001f;
    static float distanceStep = 1;

    static void Main()
    {
        Raylib.InitWindow(screenWidth, screenHeight, "Waves");
        Raylib.SetTargetFPS(60);

        while (!Raylib.WindowShouldClose())
        {
            CalculateNextField();
            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.Black);
            DrawField();
            Raylib.EndDrawing();
        }

        Raylib.CloseWindow();
    }

    static void DrawField()
    {
        for(int x = 0; x < fieldWidth; x++)
        {
            float screenX = x/fieldPerPixel;
            float screenY = screenHeight / 2 + field[x];
            
            Raylib.DrawCircle((int)screenX, (int)screenY, 5f, Color.White);
        }
    }

    static void CalculateNextField()
    {
        Array.Copy(field, oldField, fieldWidth);
        Array.Copy(newField, field, fieldWidth);

        for(int i = 0; i < fieldWidth; i++)
        {
            float newAmplitude = ComputeFieldEquation(i);
            newField[i] = newAmplitude;
        }
    }

    static float ComputeFieldEquation(int i)
    {
        // d^2f(i, n)/dt^2 = c^2 ∇^2f(i, n)
        // the acceleration of a point on the field i at timestep n is directly proportinal to the curvature of that point at that time
        // approximating d^2f/dx^2
        // f''(i, n) ∝ [f(i+di, n) - 2f(i, n) + f(i-di, n)]/di^2
        // [f(i, n+dn) - 2f(i, n) + f(i, n-dn)]/dn^2 ∝ [f(i+di, n) - 2f(i, n) + f(i-di, n)]/di^2
        // f(i, n+dn) - 2f(i, n) + f(i, n-dn) ∝ [f(i+di, n) - 2f(i, n) + f(i-di, n)] dn^2/di^2
        // f(i, n+dn)  ∝ ([f(i+di, n) - 2f(i, n) + f(i-di, n)]dn^2/di^2) + 2f(i, n) - f(i, n-dn) 
        // f(i, n+dn)  = ([f(i+di, n) - 2f(i, n) + f(i-di, n)] * c^2 * dn^2/di^2) + 2f(i, n) - f(i, n-dn) 

        float newAmplitude;
        float amplitude = field[i];
        float oldAmplitude = oldField[i];
        int leftIndex = i-1;
        if(leftIndex < 0) leftIndex = fieldWidth - 1;
        int rightIndex = i + 1;
        if(rightIndex > fieldWidth - 1) rightIndex = 0;
        float leftAmplitude = field[leftIndex];
        float rightAmplitude = field[rightIndex];

        float sourceAmount = 0;
        if(i == fieldWidth/2) sourceAmount += 2 * MathF.Sin((float)Raylib.GetTime() * 10);

        newAmplitude = 2*amplitude - oldAmplitude + C*C*(timeStep*timeStep/(distanceStep*distanceStep))*(rightAmplitude - 2*amplitude + leftAmplitude) + sourceAmount;
        
        if (i == 0 || i == fieldWidth - 1) newAmplitude = 0;
        return newAmplitude;
    }
}