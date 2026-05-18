using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using MyCppBridge;

namespace Wpf_CppCLI_Example
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            // 1. C++/CLI 클래스 인스턴스 생성
            BridgeClass bridge = new BridgeClass();



            // get
            string message = bridge.GetHelloFromCpp();
            int getint = bridge.getIntFromCpp();
            float getfloat = bridge.getfloatFromCpp();
            char getchar = bridge.getcharvalueFromCpp();

            // set
            //structSample stSample = new structSample();
            //stSample.intvalue = 42; ;
            //stSample.floatvalue = 3.14f;
            //stSample.stringValue = "from c sharp !!";
            //bridge.setStructFromCharp(ref stSample);


            // update
            int intptr = 0;
            bridge.updateIntValue(ref intptr);

            //int unsafeintptr = 0;
            //bridge.updateIntptrValue(&unsafeintptr); // unsafe 옵션 필요

            string messageInOut = "stringInOut";
            bridge.updateStringValue(ref messageInOut);


            MessageBox.Show(message, "C++/CLI 결과");
        }
    }
}