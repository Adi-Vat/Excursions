using System.Runtime.Intrinsics.X86;
using System.Text.RegularExpressions;

static class Program
{
    const int MAX_ITERATIONS = 100;

    enum Operations
    {
        Equals,
        Add,
        Subtract,
        Multiply,
        Divide,
        Juxtapose
    }

    static char[] operatorCharacters = {
        '=',
        '+',
        '-',
        '*',
        '/',
        '&',
    };

    enum TokenType
    {
        Operand,
        Leftbracket,
        Rightbracket,
        Operator
    }

    static void Main(){
        /*
        string equation = "a * d(2+3(c+b))/5";
        equation = equation.Replace(" ", "");
        equation = ApplyImplicitMultiplication(equation);
        List<string> rpnExpression = rpn(equation.Split("=")[0]);
        List<Node> nodeTree = GenerateTree(rpnExpression);

        nodeTree[nodeTree.Count - 1].PrintPretty("", true);
        */
        List<string> output = rpnFixed("2 * 3(1+4(3+5)/5)");
        List<Node> nodeTree = GenerateTree(output);
        nodeTree.Reverse();
        nodeTree[0].PrintPretty("", true);
        /*
        foreach(Node node in nodeTree)
        {
            Console.Write("[" + nodeTree.IndexOf(node) + "] : " + node.value);
            if(node.leftChild != null) Console.Write("  [" + nodeTree.IndexOf(node.leftChild)+ "]");
            if(node.rightChild != null) Console.Write(" | [" + nodeTree.IndexOf(node.rightChild)+ "]");

            Console.Write("\n");
        }*/
    }

    // returns list of nodes, last item is the head node
    static List<Node> GenerateTree(List<string> rpnExpression)
    {   
        List<Node> operatorNodes = new List<Node>();
        List<Node> allNodes = new List<Node>();
        // While there are sub-expressions to evaluate
        int ptr = 0;
        while(rpnExpression.Count() > 1)
        {
            string expressionToken = rpnExpression[ptr];

            // Reach the first operator 
            if (operatorCharacters.Contains(expressionToken[0]))
            {
                // Convert between operation symbol and name (for the enum)
                Operations operationName = (Operations)operatorCharacters.IndexOf(expressionToken[0]);
                Node newOperation = new Node(Node.NodeType.Operation, operationName.ToString());
                
                // Get the two operands associated with this operator
                string operandA = rpnExpression[ptr - 2];
                string operandB = rpnExpression[ptr - 1];

                // If either operand is a reference to a node, connect them up
                if(operandA[0] == '#')
                {
                    int nodeIndex = Int32.Parse(operandA.Substring(1));
                    newOperation.leftChild = operatorNodes[nodeIndex];
                }
                else // If its not, check if it's a variable or a constant
                {
                    Node leftChildNode = new Node(Node.NodeType.Constant, operandA);
                    newOperation.leftChild = leftChildNode;
                    allNodes.Add(leftChildNode);
                }

                if(operandB[0] == '#')
                {
                    int nodeIndex = Int32.Parse(operandB.Substring(1));
                    newOperation.rightChild = operatorNodes[nodeIndex];
                }
                else
                {
                    Node rightChildNode = new Node(Node.NodeType.Constant, operandB);
                    newOperation.rightChild = rightChildNode;
                    allNodes.Add(rightChildNode);
                }

                int operatorIndex = operatorNodes.Count();
                string expressionToAdd = "#" + operatorIndex;
                
                // Add the operator as a node to the operator nodes list
                operatorNodes.Add(newOperation);

                allNodes.Add(newOperation);

                // Remove the used sub expressions
                for(int numExpressionTokens = 0; numExpressionTokens < 3; numExpressionTokens++)
                {
                    rpnExpression.RemoveAt(ptr);
                    ptr--;
                }

                // Add the new composite sub expression
                rpnExpression.Insert(ptr + 1, expressionToAdd);

                ptr = 0;
            }
            else ptr++;
        }

        return allNodes;
    }

    static string ApplyImplicitMultiplication(string equationStr)
    {
        string newEquationStr = "";

        for(int i = 0; i < equationStr.Count(); i++)
        {
            newEquationStr += equationStr[i];
            // Check if there's an implicit multiplication
            if(i == equationStr.Count()) break;

            if(i < equationStr.Count() - 1 && equationStr[i+1] == '(' && equationStr[i] != '(')
            {
                if (!operatorCharacters.Contains(equationStr[i]))
                {
                    newEquationStr += '&';
                }
            }

            if(i > 0 && equationStr[i-1] == ')' && equationStr[i] != ')')
            {
                if (!operatorCharacters.Contains(equationStr[i]))
                {
                    newEquationStr += '&';
                }
            }
        }

        return newEquationStr;
    }

    static List<string> tokenize(string expression)
    {
        List<string> tokens = new List<string>();

        bool endOfExpression = false;
        int expressionPtr = 0;
        
        while (!endOfExpression)
        {   
            char character = expression[expressionPtr];
            
            // this doesn't work for a function name followed by a closing bracket at the end of an expression
            if(character == '$')
            {
                // Character is a function
                bool endOfFunction = false;
                int functionStrPtr = expressionPtr;
                string functionTokenStr = "";

                while (!endOfFunction)
                {
                    char functionChar = expression[functionStrPtr];
                    
                    // Reached the end of a function decleration
                    if (operatorCharacters.Contains(functionChar))
                    {
                        endOfFunction = true;
                        functionStrPtr --;
                        break;
                    }   

                    functionTokenStr += functionChar;
                    // Will eventually point to the end of the function string
                    functionStrPtr ++;

                    if(functionStrPtr == expression.Count())
                    {
                        endOfFunction = true;
                    }
                }

                // Skip the whole function
                expressionPtr = functionStrPtr;
                // Add the function to the token list
                tokens.Add(functionTokenStr);
            }
            else
            {
                tokens.Add(character.ToString());
            }

            if(expressionPtr == expression.Count() - 1) endOfExpression = true;

            expressionPtr++;
        } 

        return tokens;
    }

    static List<string> rpnFixed(string expression)
    {   
        expression = Regex.Replace(expression, @"\s", string.Empty);
        expression = ApplyImplicitMultiplication(expression);
        List<string> tokens = tokenize(expression);
        List<string> output = new List<string>();
        Stack<string> operatorStack = new Stack<string>();

        foreach(string tok in tokens)
        {
            TokenType tokenType = TokenType.Operand;

            if (operatorCharacters.Contains(tok[0])) tokenType = TokenType.Operator;    
            else if(tok == "(") tokenType = TokenType.Leftbracket;
            else if(tok == ")") tokenType = TokenType.Rightbracket;

            switch (tokenType)
            {
                case TokenType.Operand:
                    output.Add(tok);
                    break;
                case TokenType.Operator:
                    string? topOfStack;
                    operatorStack.TryPeek(out topOfStack);
                    bool emptyStack = operatorStack.Count == 0;

                    while(!emptyStack && (getPrecedence(tok) <= getPrecedence(topOfStack)) && tok != "^")
                    {
                        output.Add(operatorStack.Pop());
                        operatorStack.TryPeek(out topOfStack);
                        emptyStack = operatorStack.Count == 0;
                    }
                    
                    operatorStack.Push(tok);
                    
                    break;
                case TokenType.Leftbracket:
                    operatorStack.Push(tok);
                    break;
                case TokenType.Rightbracket:
                    emptyStack = operatorStack.Count == 0;
                    operatorStack.TryPeek(out topOfStack);

                    while(topOfStack != "(" && !emptyStack)
                    {
                        // pop and add everything inside the brackets
                        output.Add(operatorStack.Pop());
                        emptyStack = operatorStack.Count == 0;
                        operatorStack.TryPeek(out topOfStack);
                    }
                    // pop the left bracket
                    operatorStack.Pop();
                    break;
            }
        }

        while(operatorStack.Count > 0)
        {
            output.Add(operatorStack.Pop());
        }

        return output;
    }

    static int getPrecedence(string token)
    {
        if(token == "+" || token == "-") return 1;
        else if(token == "*" || token == "/") return 2;
        else if(token == "&" || token == "^") return 3;
        else return -1;
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

    public string value;

    public Node? parent;
    public Node? leftChild;
    public Node? rightChild;

    public Node(NodeType _nodeType, string _value)
    {
        nodeType = _nodeType;
        value = _value;
    }

    public void PrintPretty(string indent, bool last)
    {
        Console.Write(indent);
        if (last)
        {
            Console.Write("\\->");
            indent += " ";
        }
        else
        {
            Console.Write("|->");
            indent += "| ";
        }
        Console.WriteLine(value);

        if(leftChild != null) leftChild.PrintPretty(indent, false);
        if(rightChild != null) rightChild.PrintPretty(indent, true);
    }
}