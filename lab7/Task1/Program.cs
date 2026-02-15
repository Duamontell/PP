namespace Task1;

public static class Program
{
    public static async Task Main()
    {
        string fileName = "";
        do
        {
            Console.Write("Введите имя файла: ");
            string? input = Console.ReadLine();
            if (input != null)
            {
                fileName = input.Trim();
            }
        } while (string.IsNullOrWhiteSpace(fileName) || int.TryParse(fileName, out _));

        if (!File.Exists(fileName))
        {
            Console.WriteLine("Ошибка, данного файла не существует!");
            return;
        }

        Console.Write("Введите символы для удаления: ");
        string? symbols = Console.ReadLine();
        
        if (string.IsNullOrEmpty(symbols))
        {
            Console.WriteLine("Символы для удаления не заданы. Файл не изменён.");
            return;
        }
        
        string[] lines = await File.ReadAllLinesAsync(fileName);
        for (int i = 0; i < lines.Length; i++)
        {
            string result = lines[i];
            foreach (char symbol in symbols)
            {
                result = result.Replace(symbol.ToString(), "");
            }
            
            lines[i] = result;
            Console.WriteLine(result);
        }
        
        await File.WriteAllLinesAsync(fileName, lines);
    }
}