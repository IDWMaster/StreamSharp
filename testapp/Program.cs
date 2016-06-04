using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using StreamingVideo;
using System.IO;
namespace testapp
{
    class Program
    {
        static void Main(string[] args)
        {
            
            Video vid = new Video(File.Open("stream.mp4",FileMode.Open));
            vid.Play(IntPtr.Zero);
          //  DisplaySource src = new DisplaySource();
           // src.Record(File.Open("stream.mp4", FileMode.Create, FileAccess.Write, FileShare.ReadWrite));
        }
    }
}
