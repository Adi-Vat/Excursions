
public class Program
{
    static float frequency = MathF.PI;
    static float maxVoltage = 3.3f;
    static float Roff = 10;
    static float Ron = 1;
    static float currentState = 0f;
    static float alpha = 0.5f;
    static float beta = 0.5f;
    static float timeStep = 0.05f;
    static float temperature = 300;
    static float boltzmannConstant = 1.38E-23f;
    static float measurementBandwidth = 1;

    public static void Main()
    {
        float nextState = currentState;
        using (var fileStream = new FileStream("up_down.csv", FileMode.Create))
        using (var streamWriter = new StreamWriter(fileStream))
        {
            streamWriter.AutoFlush = true;
            float t = 0;
            Console.WriteLine("Simulating...");
            
            while (t <= 60)
            {
                currentState = nextState;
                
                float voltage = 0;
                //voltage = maxVoltage;
                //if((int)(t*2) % 2 == 0) voltage = 3.3f;
                if(t > 0 && t < 15f) voltage = maxVoltage;
                else if(t >= 15 && t < 60f) voltage = -maxVoltage;
                else voltage = 0;
                
                //voltage = maxVoltage * MathF.Sin(t * frequency);
                float memristance = ComputeMemristance(currentState);
                float current = voltage/memristance + ComputeThermalNoise(memristance);
                float dw = MolecularSwitchingRate(currentState, current) * timeStep;
                nextState = currentState + dw;
                nextState = Math.Clamp(nextState, 0, 1);
                streamWriter.WriteLine(t + "," + voltage + "," + current + "," + currentState);

                t += timeStep;
            }

            // dw/dt = i * (1-w)
            // i = v/m(w)
            // m(w) = r_off + r_on - r_off * w
            // dw = [v/(r_off + (r_on - r_off)w)] * (1-w)dt
            // dw/dt = i * w
            // dw/dt = [v/roff + (r_on - roff)w] * w
            /*
            for(float w = 0; w <= 1; w += 0.01f)
            {
                float dwBydtForward = maxVoltage/(Roff + (Ron - Roff) * w) * (1-w);
                float dwBydtBackward = maxVoltage/(Roff + (Ron - Roff) * w) * (w);
                streamWriter.WriteLine(w + "," + dwBydtForward + "," + dwBydtBackward);
            }*/

            Console.WriteLine("Done");
        }   
    }

    static float ComputeMemristance(float _currentState)
    {
        return Roff + (Ron - Roff) * _currentState;
    }

    static float MolecularSwitchingRate(float _currentState, float current)
    {
        float dwBydt = 0;
        // move towards maximum w
        if(current > 0) dwBydt = current * (1-_currentState) * alpha;
        // move towards minimum w
        else if(current < 0) dwBydt = current * _currentState * beta;
        return dwBydt;
    }

    static float ComputeThermalNoise(float resistance)
    {
        Random rng = new Random();
        double u1 = 1.0 - rng.NextDouble();
        double u2 = 1.0 - rng.NextDouble();
        double gaussian = Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Sin(2.0 * Math.PI * u2);
        double noise = gaussian * Math.Sqrt(4 * boltzmannConstant * temperature * resistance * measurementBandwidth);
        return (float)noise;
    }
}