using System.Numerics;
using System.Runtime.InteropServices;
using Raylib_cs;

class Program
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    static List<Panel> panels = new List<Panel>();
    static Panel? heldPanel;
    static Vector2 heldPanelOffsetFromMouse;

    static void Start()
    {
        Panel mainPanel = new Panel(new Vector2(20, 20), 400, 300);
        mainPanel.values.Add("hello", "world!");

        panels.Add(mainPanel);
    }

    public static void Main()
    {
        Start();

        Raylib.InitWindow(screenWidth, screenHeight, "ASCui");

        while (!Raylib.WindowShouldClose())
        {
            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.Black);
           
            Update();

            Raylib.EndDrawing();
        }
    }

    static void Update()
    {
        if(Raylib.IsMouseButtonPressed(MouseButton.Left)) TryGrabPanel();
        else if(Raylib.IsMouseButtonReleased(MouseButton.Left)) TryDropPanel();

        MovePanel();

        foreach(Panel Panel in panels)
        {
            Vector2 PanelPos = worldToScreenPosition(Panel.position);
            Raylib.DrawRectangle((int)PanelPos.X, (int)PanelPos.Y, Panel.width, Panel.height, Color.Gray);
            int numVerticalBars = 0;
            numVerticalBars = (int)Math.Floor((double)Panel.height / 16);
            for(int i = 0; i < numVerticalBars; i++)
            {
                Raylib.DrawText("|", (int)PanelPos.X, (int)PanelPos.Y + 5 +numVerticalBars * 16, 16, Color.White);
            }
        }
    }

    static void TryGrabPanel()
    {   
        if (heldPanel == null)
        {
            Vector2 mousePos = Raylib.GetMousePosition();
            // Try grab a Panel
            foreach(Panel panel in panels)
            {
                Vector2 winTopLeftCorner = panel.position;
                Vector2 winLowerRightCorner = panel.position + new Vector2(panel.width, panel.height);
                
                if(winTopLeftCorner.X <= mousePos.X && mousePos.X < winLowerRightCorner.X)
                {
                    if(winTopLeftCorner.Y <= mousePos.Y && mousePos.Y < winLowerRightCorner.Y)
                    {
                        heldPanel = panel;
                        heldPanelOffsetFromMouse = heldPanel.position - mousePos;
                    }
                }
            }
        }
    }

    static void MovePanel()
    {
        if(heldPanel != null)
        {
            Vector2 mousePos = Raylib.GetMousePosition();
            // Keep mouse offset
            
            heldPanel.position = mousePos + heldPanelOffsetFromMouse;
        }
    }

    static void TryDropPanel()
    {
        if (heldPanel != null) heldPanel = null;
    }

    static Vector2 worldToScreenPosition(Vector2 worldPos)
    {
        Vector2 screenPos = worldPos;

        return screenPos;
    }
}

class Panel
{
    public Vector2 position;
    public int width;
    public int height;

    public Dictionary<string, string> values = new Dictionary<string, string>();

    public Panel(Vector2 _position, int _width, int _height)
    {
        position = _position;
        width = _width;
        height = _height;
    }
}
