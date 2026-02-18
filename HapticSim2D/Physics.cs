
using System.Numerics;
using Raylib_cs;

public class Physics
{
    static (Vector2 centre, float width, float height) GetWallAABBCentre(Wall wall)
    {
        float maxLeft = wall.vertices[0].position.X;
        float maxRight = wall.vertices[0].position.X;
        float maxUp = wall.vertices[0].position.Y;
        float maxDown = wall.vertices[0].position.Y;

        foreach(Vertex vertex in wall.vertices)
        {
            if(vertex.position.X < maxLeft) maxLeft = vertex.position.X;
            if(vertex.position.X > maxRight) maxRight = vertex.position.X;
            if(vertex.position.Y < maxUp) maxUp = vertex.position.Y;
            if(vertex.position.Y > maxDown) maxDown = vertex.position.Y;
        }

        float width = MathF.Abs(maxRight - maxLeft);
        float height = MathF.Abs(maxDown - maxUp);
            
        Vector2 aabbCenter = new Vector2((maxLeft + maxRight) / 2, (maxUp + maxDown) / 2) + wall.position;
        return (aabbCenter, width, height);
    }
    // Returns the normal force applied on the ball from the wall
    public static (Vector2 normalForce, List<Wall> nextWalls) ApplyBallPositionToMouse(Ball ball, Vector2 mousePosition, List<Wall> walls)
    {
        // --- Ball stuff ---
        Vector2 dirBallToMouse = Vector2.Normalize(mousePosition - ball.position);
        if(float.IsNaN(dirBallToMouse.X)) dirBallToMouse.X = 0;
        if(float.IsNaN(dirBallToMouse.Y)) dirBallToMouse.Y = 0;
        float distanceBallToMouse = Vector2.Distance(mousePosition, ball.position);

        Vector2 normalForce = Vector2.Zero;
        List<Wall> nextWalls = walls.ToList();

        foreach(Wall wall in walls)
        {
            (Vector2 aabbCentre, float width, float height) = GetWallAABBCentre(wall);
  
            if(CheckAABB(ball.position, ball.radius * 2, ball.radius * 2, aabbCentre, width, height))
            {
                for(int i = 0; i < wall.vertices.Count(); i++)
                {   
                    if(i < wall.vertices.Count() - 1)
                    {
                        Vector2 lineSegPointA = wall.vertices[i].position + wall.position;
                        Vector2 lineSegPointB = wall.vertices[i+1].position + wall.position;
                        (Vector2 tempNormalForce, float t, Vector2 closestPoint) = GetLineSegNormalForce(ball, lineSegPointA, lineSegPointB);
                        
                        if(tempNormalForce.Length() > normalForce.Length()){
                            normalForce = tempNormalForce;
                            // Apply force to whole line segment (2 verts) based on the distance to each end
                            Vector2 forceA = -normalForce * (1-t) - (wall.vertices[i].velocity * 10);
                            Vector2 forceB = -normalForce * (t) - (wall.vertices[i+1].velocity * 10);
                            wall.vertices[i].velocity += forceA * Raylib.GetFrameTime();
                            wall.vertices[i+1].velocity += forceB * Raylib.GetFrameTime();
                        }

                        if(tempNormalForce.Length() >= Program.forceCutThreshold && Program.cutTimer == 0)
                        {
                            Vector2 cutPoint = closestPoint - wall.position;
                            Vertex cutVertexLeft = new Vertex(cutPoint);
                            cutVertexLeft.bondLength = Vector2.Distance(cutPoint, wall.vertices[i].position);
                            Vertex cutVertexRight = new Vertex(cutPoint);
                            cutVertexRight.bondLength = Vector2.Distance(cutPoint, wall.vertices[i+1].position);
                            
                            List<Vertex> leftWallVerts = new List<Vertex>();
                            for(int k = 0; k <= i; k++) leftWallVerts.Add(wall.vertices[k]);

                            List<Vertex> rightWallVerts = new List<Vertex>();
                            for(int k = wall.vertices.Count - 1; k > i; k--) rightWallVerts.Add(wall.vertices[k]);

                            leftWallVerts.Add(cutVertexLeft);
                            Wall newLeftWall = new Wall(wall.position);

                            rightWallVerts.Add(cutVertexRight);
                            Wall newRightWall = new Wall(wall.position);

                            newLeftWall.vertices = leftWallVerts.ToList();
                            newRightWall.vertices = rightWallVerts.ToList();
                            nextWalls.Add(newLeftWall);
                            nextWalls.Add(newRightWall);
                            nextWalls.Remove(wall);
                            Program.cutTimer = Program.cutWaitTime;
                        }
                    }
                }
            }
        }

        Vector2 force = ball.springConstant * dirBallToMouse * distanceBallToMouse - (ball.velocity * ball.dampingFactor);
        force += normalForce;
        Vector2 acceleration = force/ball.mass;
        ball.velocity += acceleration * Raylib.GetFrameTime();
        ball.position += ball.velocity * Raylib.GetFrameTime();
        return (normalForce, nextWalls);
    }

    static (Vector2 normalForce, float t, Vector2 closestPosition) GetLineSegNormalForce(Ball ball, Vector2 lineSegPointA, Vector2 lineSegPointB)
    {
        Vector2 normalForce = Vector2.Zero;
        float t = Vector2.Dot(ball.position - lineSegPointA, lineSegPointB - lineSegPointA) /
                    Vector2.Dot(lineSegPointB - lineSegPointA, lineSegPointB - lineSegPointA);

        
        t = Math.Clamp(t, 0, 0.99f);
        
        Vector2 closestPoint = lineSegPointA + t * (lineSegPointB - lineSegPointA);
        
        Vector2 normal = Vector2.Normalize(ball.position - closestPoint);
        float distance = (ball.position - closestPoint).Length();
        float depth = ball.radius - distance;
        
        if(float.IsNaN(normal.X)) normal.X = 0;
        if(float.IsNaN(normal.Y)) normal.Y = 0;

        if(distance <= ball.radius) normalForce = normal * depth * 2000;
        
        return (normalForce, t, closestPoint);
    }

    public static bool CheckAABB(Vector2 centreA, float widthA, float heightA, Vector2 centreB, float widthB, float heightB)
    {
        float minX_A = centreA.X - widthA/2;
        float maxX_A = centreA.X + widthA/2;

        float minY_A = centreA.Y - heightA/2;
        float maxY_A = centreA.Y + heightA/2;

        float minX_B = centreB.X - widthB/2;
        float maxX_B = centreB.X + widthB/2;

        float minY_B = centreB.Y - heightB/2;
        float maxY_B = centreB.Y + heightB/2;

        if(maxX_A < minX_B) return false;
        if(maxX_B < minX_A) return false;
        if(maxY_A < minY_B) return false;
        if(maxY_B < minY_A) return false;

        return true;
    }
}