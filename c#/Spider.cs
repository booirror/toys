using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace MailSpider
{
    class Spider
    {
        private string[] symbol = null;
        private int waitIdx = 0;
        private bool isOver = false;
        private Queue outQueue = null;//输出消息队列
        private Dictionary<string, string> dict = null;//存放URL
        private HashSet<string> mails = null;//存放邮箱
        private HashSet<string> marks = null;//存放已经爬取过的URL
        private string[] filenames = null;
        private int len = 0;
        private int count = 0;

        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Useage: {0} url [site]", Process.GetCurrentProcess().MainModule.ModuleName);
                return;
            }
            new Spider().CrawlMail(args[0], args.Length > 1 ? args[1] : null);
        }

        public Spider()
        {
            InitData();
        }

        private void InitData()
        {
            len = 0;
            count = 0;
            waitIdx = 0;
            isOver = false;
            symbol = new string[] { "|", "/", "-", "\\" };
            filenames = new string[] { ".zip", ".rar", ".apk", ".mp3", ".mp4", ".swf"};
            outQueue = Queue.Synchronized(new Queue());
            dict = new Dictionary<string, string>();
            mails = new HashSet<string>();
            marks = new HashSet<string>();
        }

        private void AddOutputString(string str)
        {
            outQueue.Enqueue(str);
        }

        private void AddOutputMail(string mail)
        {
            AddOutputString("m" + mail);
        }

        private void AddOutputUrl(string url)
        {
            AddOutputString("u" + url);
        }

        private string GetOutputString()
        {
            if (outQueue.Count == 0) return null;
            return outQueue.Dequeue() as string;
        }

        private void ConsoleClear()
        {
            Console.CursorLeft = 0;
            Console.CursorTop = Console.CursorTop - (len - 1) / Console.BufferWidth;
            string space = new string(' ', len);
            Console.Write(space);
            if (len > 0 && len % Console.BufferWidth == 0)
            {
                Console.CursorTop = Console.CursorTop - 1;
            }
            Console.CursorLeft = 0;
            Console.CursorTop = Console.CursorTop - (len - 1) / Console.BufferWidth;
        }

        private void Output()
        {
            while(!isOver)
            {
                
                string s = GetOutputString();
                if (s == null)
                {
                    Console.Write(symbol[waitIdx]);
                    if (Console.CursorLeft == 0)
                    {
                        Console.CursorLeft = Console.BufferWidth-1;
                        Console.CursorTop = Console.CursorTop - 1;
                    }
                    else
                    {
                        Console.CursorLeft = Console.CursorLeft - 1;
                    }
                    waitIdx++;
                    if (waitIdx >= symbol.Length)
                    {
                        waitIdx = 0;
                    }
                }
                else
                {
                    if (s[0] == 'm') {
                        ConsoleClear();
                        count++;
                        Console.WriteLine("第 {0} 个： {1}", count, s.Substring(1));
                        len = 0;
                    } else if (s[0] == 'u') {
                        ConsoleClear();
                        string buff = string.Format("scanning {0} ", s.Substring(1));
                        len = Encoding.Default.GetBytes(buff).Length + 1;
                        Console.Write(buff);
                    } else {
                        Console.WriteLine(s);
                    }
                }
                
                Thread.Sleep(200);
            }
        }

        private void CrawlMail(string url, string site)
        {
            Console.BackgroundColor = ConsoleColor.Black;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(@"爬虫开始行动了，所有邮件地址将保存在""mails.txt""里。");
            Thread thread = new Thread(Output);
            thread.Start();
            dict.Add(url, null);
            while (dict.Count > 0)
            {
                var item = dict.First();
                dict.Remove(item.Key);
                marks.Add(item.Key);
                if (site == null || item.Key.Contains(site))
                {
                    string data = DownloadPage(item.Key, item.Value);
                    if (data != null)
                    {
                        int num = FetchAllMail(data);
                        FetchAllUrl(data, item.Key, num);
                    }
                }
            }
            isOver = true;
            SaveMailData();
            Console.WriteLine("\r\ntotal number: {0}, bye!", mails.Count);
            Thread.Sleep(1000);
        }
        /// <summary>
        /// 保存邮箱地址到mails.txt中
        /// </summary>
        private void SaveMailData()
        {
            FileStream fs = new FileStream("mails.txt", FileMode.Append, FileAccess.Write);
            StreamWriter sw = new StreamWriter(fs);
            foreach (string mail in mails)
            {
                sw.WriteLine(mail);
            }
            sw.Close();
            fs.Close();
        }

        /// <summary>
        /// 求URL的绝对路径
        /// </summary>
        /// <param name="url">页面里的url</param>
        /// <param name="srcUrl">页面URL</param>
        /// <returns></returns>
        private string AbsoluteUrl(string url, string srcUrl)
        {
            Regex http = new Regex(@"^http");
            Match m = http.Match(url);
            if (m.Success)
                return url;
            if (srcUrl[srcUrl.Length - 1] == '/')
            {
                srcUrl = srcUrl.Substring(0, srcUrl.Length - 1);
            }
            string newUrl = srcUrl.Substring(0, srcUrl.LastIndexOf('/')+1);
            if (url[0] == '/')
            {
                Regex regex = new Regex("(?<root>https?://.*?)/");
                Match match = regex.Match(newUrl);
                if (match.Success)
                {
                    return match.Result("${root}") + url;
                }
            }
            return newUrl + url;
        }
        /// <summary>
        /// 判断是否是HTML，filenames存放了不是一个页面链接的后缀，不完整，目前够用
        /// </summary>
        /// <param name="url"></param>
        /// <returns></returns>
        private bool IsHtml(string url)
        {
            if (url.Length < 7)
                return false;
            string name = url.Substring(url.Length - 4);
            foreach (string f in filenames) {
                if (f.Equals(name))
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// 提取页面里的所有的URL
        /// </summary>
        /// <param name="data"></param>
        /// <param name="srcUrl"></param>
        /// <param name="num"></param>
        private void FetchAllUrl(string data, string srcUrl, int num)
        {
            Regex regex = new Regex(@"<a href=['""](?<url>[^\s]+?)['""]");
            MatchCollection mc = regex.Matches(data);
            if (mc.Count > 0) {
                foreach (Match m in mc) {
                    string url = m.Result("${url}");
                    if (num > 5)
                    {
                        url = AbsoluteUrl(url, srcUrl);
                    }
                    if (!marks.Contains(url) && !dict.ContainsKey(url) && IsHtml(url)) {
                        dict.Add(url, srcUrl);
                    }
                }
            }
        }

        /// <summary>
        /// 提取所有邮件地址
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        private int FetchAllMail(string data)
        {
            Regex regex = new Regex(@"(?<mail>[A-Za-z\d][A-Za-z\d\._]*@[A-Za-z\d\.]+\.[A-Za-z]+)");
            MatchCollection mc = regex.Matches(data);
            if (mc.Count > 0)
            {
                foreach (Match m in mc)
                {
                    string mail = m.Result("${mail}");
                    if (!mails.Contains(mail))
                    {
                        mails.Add(mail);
                        AddOutputMail(mail);
                    }
                }
            }
            return mc.Count;
        }

        /// <summary>
        /// 下载页面
        /// </summary>
        /// <param name="url"></param>
        /// <param name="from"></param>
        /// <returns></returns>
        private string DownloadPage(string url, string from)
        {
            AddOutputUrl(url);
            HttpWebResponse response = null;
            StreamReader streamReader = null;
            string data = null;
            try
            {
                HttpWebRequest request = WebRequest.Create(url) as HttpWebRequest;
                request.Credentials = new CredentialCache();
                request.Method = "GET";
                request.UserAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.80 Safari/537.36";
                if (from != null)
                {
                    request.Referer = from;
                }
                request.Timeout = 6000;
                request.ReadWriteTimeout = 6000;
                response = request.GetResponse() as HttpWebResponse;
                streamReader = new StreamReader(response.GetResponseStream());
                data = streamReader.ReadToEnd();
                streamReader.Close();
                response.Close();
            }
            catch (Exception e)
            {
                if (response != null)
                {
                    response.Close();
                }
                if (streamReader != null)
                {
                    streamReader.Close();
                }
                return null;
            }
            return data;
        }
    }
}
