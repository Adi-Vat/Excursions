using System.Runtime.Intrinsics.X86;

static class Program
{
    const int MAX_ITERATIONS = 100;

    enum Operations
    {
        Equals,
        Add,
        Subtract,
        Multiply,
        Divide
    }

    static char[] operatorCharacters = {
        '=',
        '+',
        '-',
        '*',
        '/',
        '(',
        ')'
    };

    static int[] precedence =
    {
        0,
        1,1,
        2,2,
        3,3
    };

    static void Main(){
        //string? equationStr = Console.ReadLine();
        //DecomposeEquation(equationStr);
        //string val = SplitBrackets("a*(2+3(c+b))");
        //(string val, string other) = ExtractBracketExpression("a*(2+3(c+b))", 0);
        //Console.WriteLine(val + "\n" + other);
        //string expression = "a*(2+3(c+b))";
        //expression = FixImpliedMultiplication(expression);
        //string outValue = SplitBrackets(expression, 0);
        string rpnExpression = rpn("3+5/(6*(7+2))"); 
        GenerateTree(rpnExpression);
    }

    static void ConvertEquationToTree(string equation)
    {
        Node equ = new Node(Node.NodeType.Operation, (int)Operations.Equals);
        string lhs = equation.Split("=")[0];
        string rhs = equation.Split("=")[1];
    }

    static Node GenerateTree(string rpnExpression)
    {
        Node headNode = new Node(Node.NodeType.Operation, (int)Operations.Equals);

        Stack<string> opStack = new Stack<string>(); 

        for(int i = 0; i < rpnExpression.Count(); i++)
        {
            char element = rpnExpression[i];
            if(!operatorCharacters.Contains(element))
            {
                
            }
        }

        return headNode;
    }

    static string rpn(string expression)
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

        
        Queue<string> outputQueue = new Queue<string>();
        Stack<string> operatorStack = new Stack<string>();

        // Only parses 1 character variables
        for(int i = 0; i < tokens.Count(); i++)
        {
            char character = expression[i];
            
            // character is an operand
            if(operatorCharacters.Contains(character))
            {
                char o1 = character;
                if(operatorStack.Count > 0)
                {
                    char o2 = operatorStack.Peek()[0];
                    int deltaPrecedence = precedence[operatorCharacters.IndexOf(o1)] - precedence[operatorCharacters.IndexOf(o2)];
                    // p > 0; o1 > o2
                    // p = 0; o1 = o2
                    // p < 0; o2 > o1
                    bool keepGoing = o2 != '(' && (deltaPrecedence < 0 || (deltaPrecedence == 0 && o1 != '^'));
                    // Character is an operator
                    while (keepGoing)
                    {
                        operatorStack.Pop();
                        outputQueue.Enqueue(o2.ToString());
                        keepGoing = false;
                        if(operatorStack.Count > 0)
                        {
                            o2 = operatorStack.Peek()[0];
                            deltaPrecedence = precedence[operatorCharacters.IndexOf(o1)] - precedence[operatorCharacters.IndexOf(o2)];
                            keepGoing = o2 != '(' && (deltaPrecedence < 0 || (deltaPrecedence == 0 && o1 != '^'));
                        }
                    }
                }
                operatorStack.Push(o1.ToString());
            } 
            else if(character == '(')
            {
                operatorStack.Push(character.ToString());
            } 
            else if(character == ')')
            {
                char o2 = operatorStack.Peek()[0];
                while(o2 != '(' && operatorStack.Count() > 0)
                {
                    operatorStack.Pop();
                    outputQueue.Enqueue(o2.ToString());    
                }

                // There must be a left parenthesis at the top of the operator stack
                operatorStack.Pop();
                // deal with functions here 
            }
            else
            {
                outputQueue.Enqueue(character.ToString());
            }
        }

        while(operatorStack.Count() > 0)
        {
            string o1 = operatorStack.Pop();
            outputQueue.Enqueue(o1);
        }

        string outputExpression = "";
        foreach(string element in outputQueue)
        {
            if(element != "(" && element != ")") outputExpression += element;
        }

        return outputExpression;
    }

    static void DecomposeEquation(string? equationStr)
    {
        if(equationStr == null) return;
        // Remove whitespace
        equationStr = equationStr.Replace(" ", "");
        string newEquationStr = "";

        for(int i = 0; i < equationStr.Count(); i++)
        {
            newEquationStr += equationStr[i];
            // Check if there's an implicit multiplication
            if(i == equationStr.Count() - 1) break;

            if(equationStr[i+1] == '(')
            {
                if (!operatorCharacters.Contains(equationStr[i]))
                {
                    newEquationStr += '*';
                }
            }
        }

        string[] expressions = newEquationStr.Split("=");
        if(expressions.Count() != 2) return;
        
        string lhs = expressions[0];
        string rhs = expressions[1];

        

        // Break down LHS
        /*
        while (!lhs.IsWhiteSpace())
        {
            
        }*/

        

        // Compute LHS
        bool lhsSimplified = false;
        int iterations = 0;
        /*
        while (!lhsSimplified)
        {
            if(iterations > MAX_ITERATIONS) break;

            iterations ++;
        }
        */
    }

    static string FixImpliedMultiplication(string equationStr)
    {
        string newEquationStr = "";

        for(int i = 0; i < equationStr.Count(); i++)
        {
            newEquationStr += equationStr[i];
            // Check if there's an implicit multiplication
            if(i == equationStr.Count() - 1) break;

            if(equationStr[i+1] == '(')
            {
                if (!operatorCharacters.Contains(equationStr[i]))
                {
                    newEquationStr += '*';
                }
            }
        }

        return newEquationStr;
    }

    static string SplitBrackets(string expression, int bracketIndex)
    {
        // a * (2+3(c+b))
        // a * $0
        // $0 = 2+3(c+b)
        // $0 = 2+3*$2
        // $1 = c+b
        if(!expression.Contains('(')) return expression;

        (string bracketExpression, string outsideExpression) = ExtractBracketExpression(expression, bracketIndex);
        Console.WriteLine(outsideExpression + ", $" +bracketIndex + " = "+ bracketExpression);
        return SplitBrackets(bracketExpression, bracketIndex + 1); 
    }

    static (string bracket,string outsideBracket) ExtractBracketExpression(string expression, int bracketIndex)
    {
        // a * (b + c) + d
        int bracketsNum = 0;
        int openBracketIndex = 0;
        int closeBracketIndex = 0;
        // a * [(]b + c[)] + d
        for(int i = 0; i < expression.Count(); i++)
        {
            if(expression[i] == '(')
            {
                if(bracketsNum == 0) openBracketIndex = i; 
                bracketsNum++;    
            }
            else if(expression[i] == ')')
            {
                bracketsNum--;
                if(bracketsNum == 0) closeBracketIndex = i;
            }    
        }

        //Console.WriteLine(openBracketIndex + "  " + closeBracketIndex);

        // No brackets
        //if ((openBracketIndex | closeBracketIndex) == 0) return("", expression);

        string bracketStr = expression.Substring(openBracketIndex+1, closeBracketIndex-openBracketIndex-1);
        
        string remainderStr = expression;
        if(bracketStr != "") remainderStr = expression.Replace(bracketStr, "$"+bracketIndex);
        remainderStr = remainderStr.Replace("(", "");
        remainderStr = remainderStr.Replace(")", "");

        return (bracketStr, remainderStr);
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

    public int value;

    public Node? parent;
    public Node? leftChild;
    public Node? rightChild;

    public Node(NodeType _nodeType, int _value)
    {
        nodeType = _nodeType;
        value = _value;
    }
}