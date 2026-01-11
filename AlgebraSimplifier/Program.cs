static class Program
{
    const int MAX_ITERATIONS = 100;

    static void Main(){
        string? equationStr = Console.ReadLine();
        DecomposeEquation(equationStr);
    }

    static void DecomposeEquation(string? equationStr)
    {
        if(equationStr == null) return;
        // Remove whitespace
        equationStr = equationStr.Replace(" ", "");
        
        string[] expressions = equationStr.Split("=");
        if(expressions.Count() != 2) return;
        
        string lhs = expressions[0];
        string rhs = expressions[1];

        // Break down LHS
        while (!lhs.IsWhiteSpace())
        {
            
        }

        // Compute LHS
        bool lhsSimplified = false;
        int iterations = 0;
        while (!lhsSimplified)
        {
            if(iterations > MAX_ITERATIONS) break;



            iterations ++;
        }
    }
}

class Node
{
    public enum NodeType
    {
        Constant,
        Variable,
        Operation
    }

    public NodeType nodeType;

    public Node? parent;
    public Node? leftChild;
    public Node? rightChild;

    public Node(Node? _parent, Node? _leftChild, Node? _rightChild)
    {
        parent = _parent;
        leftChild = _leftChild;
        rightChild = _rightChild;
    }
}

class ExpressionTree
{
    List<Node> nodes = new List<Node>();
    Node headNode;

    public enum Side
    {
        Left,
        Right
    }

    public ExpressionTree(Node _headNode)
    {
        headNode = _headNode;
    }

    public void AddNode(Node _node, Node _parent, Side _side)
    {
        switch (_side)
        {
            case Side.Left:
            _parent.leftChild = _node;
                break;
            case Side.Right:
            _parent.rightChild = _node;
                break;
        }

        nodes.Add(_node);
    }

    // Removes that node and all nodes under it
    public void RemoveNode(Node _node)
    {
        nodes.Remove(_node);

        if(_node.parent?.leftChild == _node)
        {
            
        } else if(_node.parent?.rightChild == _node)
        {
            
        }
    }
}