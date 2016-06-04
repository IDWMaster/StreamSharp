using StreamingVideo;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms.Integration;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace VideoStreamSample
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        WindowsFormsHost host = new WindowsFormsHost();
        public MainWindow()
        {


            
            InitializeComponent();
            
            host.Child = new System.Windows.Forms.Control();

            grid.Children.Add(host);
            var hwnd = host.Child.Handle;
            Video vid = new Video(null);
            vid.Play(hwnd);
        }

     
    }
}
