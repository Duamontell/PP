using System.Diagnostics;
using System.Text.Json;

namespace Task2;

public static class Program
{
    private const string ApiUrl = "https://dog.ceo/api/breeds/image/random";
    private const string ApiKey = "message";
    private const int Count = 10;

    public static async Task Main()
    {
        Stopwatch stopwatch = new Stopwatch();
        stopwatch.Start();

        using HttpClient client = new();
        List<Task> tasks = new();

        for (int i = 0; i < Count; i++)
        {
            int index = i;
            tasks.Add(Task.Run(async () =>
            {
                try
                {
                    using HttpResponseMessage result = await client.GetAsync(ApiUrl);
                    if (!result.IsSuccessStatusCode)
                    {
                        Console.WriteLine($"{index + 1}: API вернул статус {(int)result.StatusCode} ({result.ReasonPhrase}).");
                        return;
                    }
                    string json = await result.Content.ReadAsStringAsync();

                    using JsonDocument imageUrlJson = JsonDocument.Parse(json);
                    if (!imageUrlJson.RootElement.TryGetProperty(ApiKey, out var imageUrl))
                    {
                        return;
                    }

                    string url = imageUrl.GetString() ?? "";
                    Console.WriteLine($"Скачиваем картинку: {url}");

                    byte[] imageBytes = await client.GetByteArrayAsync(url);
                    string fileName = $"{index + 1}.jpg";

                    await File.WriteAllBytesAsync(fileName, imageBytes);
                    Console.WriteLine($"Картинка {index + 1} сохранена успешно!");
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Ошибка у потока {index + 1}: {e.Message}");
                }
            }));
        }

        await Task.WhenAll(tasks);

        Console.WriteLine("Все загрузки завершены.");
        stopwatch.Stop();
        Console.WriteLine(stopwatch.Elapsed.TotalMilliseconds);
    }
}