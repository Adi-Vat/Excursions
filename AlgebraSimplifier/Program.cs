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
        '@',
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
        List<string> output = rpnFixed("2(a+b+c)");
        List<Node> nodeTree = GenerateTree(output);
        ApplySimplificationRules(nodeTree);
        nodeTree[0].PrintPretty("", true);
        //ApplySimplificationRules(nodeTree);
        /*
        foreach(Node node in nodeTree)
        {
            Console.Write("[" + nodeTree.IndexOf(node) + "] : " + node.value);
            if(node.leftChild != null) Console.Write("  [" + nodeTree.IndexOf(node.leftChild)+ "]");
            if(node.rightChild != null) Console.Write(" | [" + nodeTree.IndexOf(node.rightChild)+ "]");

            Console.Write("\n");
        }*/
    }

    static (List<Node> simplifiedTree, bool wasSimplified) ApplySimplificationRules(List<Node> allNodes)
    {
        List<Node> simplifiedTree = new List<Node>();
        bool wasSimplified = false;

        // Apply simplification rules
        // RULE 1: associate
        
        return (simplifiedTree, wasSimplified);
    }

    // returns list of nodes, last item is the head node
    static List<Node> GenerateTree(List<string> rpnExpression)
    {   
        List<Node> operatorNodes = new List<Node>();
        List<Node> allNodes = new List<Node>();
        // While there are sub-expressions to evaluate
        int ptr = 0;
        
        Console.Write("\n");
        foreach(string tok in rpnExpression) Console.Write(tok);
        Console.Write("\n");

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
                    Node.NodeType thisNodeType = Node.NodeType.Variable;

                    int val = 0;
                    if(Int32.TryParse(operandA, out val))
                    {
                        thisNodeType = Node.NodeType.Constant;
                    }

                    Node leftChildNode = new Node(thisNodeType, operandA);
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
                    Node.NodeType thisNodeType = Node.NodeType.Variable;

                    int val = 0;
                    if(Int32.TryParse(operandB, out val))
                    {
                        thisNodeType = Node.NodeType.Constant;
                    }

                    Node rightChildNode = new Node(thisNodeType, operandB);
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

        // Reverse so that the head node is the first item in the list
        allNodes.Reverse();
        return allNodes;
    }

    static List<string> ApplyImplicitMultiplication(List<string> equationList)
    {
        List<string> newEquationList = new List<string>();

        for(int i = 0; i < equationList.Count(); i++)
        {
            newEquationList.Add(equationList[i]);

            // Check behind
            // Apply implicit multiplication only when:
            // (a+b)(b+c) [maybe also remove un-necessary brackets in pre-processing]
            // > right - left bracket back to back ')('
            // 2a
            // > constant and variable juxtaposed
            // 3(a + b), a(a + b)
            // > constant/variable next to left backet
            // Two different items next to each other, 2a, 3(, )a, a2, )(
            // or they're both variables, ab, bc etc 
            // but not (a, a), )), ((
            if(i < equationList.Count - 1)
            {
                TokenType thisTokenType = getTokenType(equationList[i]);
                TokenType nextTokenType = getTokenType(equationList[i + 1]);

                bool applyMutliplier = false;
        
                if(thisTokenType != nextTokenType) applyMutliplier = true;
                else if(thisTokenType == TokenType.Operand && nextTokenType == TokenType.Operand) applyMutliplier = true;

                if(thisTokenType == TokenType.Operator || nextTokenType == TokenType.Operator) applyMutliplier = false;
                if(thisTokenType == TokenType.Leftbracket) applyMutliplier = false;
                if(nextTokenType == TokenType.Rightbracket) applyMutliplier = false;

                if(applyMutliplier) newEquationList.Add("@");
            }
        }
        Console.Write("\n");
        foreach(string token in newEquationList) Console.Write(token);
        Console.Write("\n");
        return newEquationList;
    }

    static TokenType getTokenType(string tok)
    {
        TokenType tokenType = TokenType.Operand;

        if (operatorCharacters.Contains(tok[0])) tokenType = TokenType.Operator;    
        else if(tok == "(") tokenType = TokenType.Leftbracket;
        else if(tok == ")") tokenType = TokenType.Rightbracket;

        return tokenType;
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
            else if (character != ' ')
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
        //expression = Regex.Replace(expression, @"\s", string.Empty);
        List<string> tokens = tokenize(expression);
        tokens = ApplyImplicitMultiplication(tokens);

        List<string> output = new List<string>();
        Stack<string> operatorStack = new Stack<string>();
    
        foreach(string tok in tokens)
        {
            TokenType tokenType = getTokenType(tok);

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
        else if(token == "@" || token == "^") return 3;
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