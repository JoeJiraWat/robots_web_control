import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:provider/provider.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import 'package:http/http.dart' as http;

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  SystemChrome.setPreferredOrientations([
    DeviceOrientation.landscapeLeft,
    DeviceOrientation.landscapeRight,
  ]).then((_) {
    runApp(const MyApp());
  });
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Robot Control',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: ChangeNotifierProvider(
        create: (context) => RobotController(),
        child: const MyHomePage(),
      ),
    );
  }
}

class RobotController with ChangeNotifier {
  final String _robotIp = '192.168.4.1';
  String _wifiStatus = 'Unknown';
  String _httpStatus = 'Awaiting command...';
  
  String get wifiStatus => _wifiStatus;
  String get httpStatus => _httpStatus;

  // Debounce to prevent flooding the server with requests
  DateTime _lastCommandTime = DateTime.now();
  final Duration _debounceDuration = const Duration(milliseconds: 200);

  Future<void> checkInitialConnection() async {
    _wifiStatus = 'Checking...';
    notifyListeners();
    try {
      final url = Uri.http(_robotIp, '/stop');
      final response = await http.get(url).timeout(const Duration(seconds: 2));
      if (response.statusCode == 200) {
        _wifiStatus = 'Connected';
        _httpStatus = 'Ready';
      } else {
        _wifiStatus = 'Disconnected';
        _httpStatus = 'Check failed';
      }
    } catch (e) {
      _wifiStatus = 'Disconnected (Check WiFi)';
      _httpStatus = 'Check failed';
      print('Error on initial connection: $e');
    }
    notifyListeners();
  }

  Future<void> _sendCommand(String command, {String params = ''}) async {
    final now = DateTime.now();
    if (now.difference(_lastCommandTime) < _debounceDuration && command != 'stop') {
      return;
    }
    _lastCommandTime = now;

    try {
      final url = Uri.http(_robotIp, '/$command', params.isNotEmpty ? {'pos': params} : null);
      _httpStatus = 'Sending "$command"...';
      notifyListeners();

      final response = await http.get(url).timeout(const Duration(seconds: 1));

      if (response.statusCode == 200) {
        _httpStatus = 'OK';
        if (_wifiStatus != 'Connected') _wifiStatus = 'Connected';
      } else {
        _httpStatus = 'Error: ${response.statusCode}';
        if (_wifiStatus != 'Connected') _wifiStatus = 'Connected';
      }
    } catch (e) {
      _httpStatus = 'Request Failed';
      _wifiStatus = 'Disconnected (Check WiFi)';
      print('Error sending command: $e');
    }
    notifyListeners();
  }

  double _servo1Value = 90;
  bool _servo2Toggled = false;

  double get servo1Value => _servo1Value;
  bool get servo2Toggled => _servo2Toggled;

  void moveServo1(StickDragDetails details) {
    double value = _servo1Value;
    // Adjust sensitivity by changing the increment value
    if (details.y < -0.1) {
      value += 5; 
      if (value > 180) value = 180;
    } else if (details.y > 0.1) {
      value -= 5;
      if (value < 0) value = 0;
    }

    int intValue = value.round();
    if (intValue != _servo1Value.round()) {
      _servo1Value = value;
      _sendCommand('servo1', params: intValue.toString());
      notifyListeners();
    }
  }

  void toggleServo2() {
    _servo2Toggled = !_servo2Toggled;
    _sendCommand('servo2', params: _servo2Toggled ? '180' : '0');
    notifyListeners();
  }

  void move(StickDragDetails details) {
    double dx = details.x;
    double dy = details.y;

    if (dy.abs() < 0.2 && dx.abs() < 0.2) {
      _sendCommand('stop');
      return;
    }

    if (dy < -dx && dy < dx) {
        _sendCommand('forward');
    } else if (dy > -dx && dy > dx) {
        _sendCommand('backward');
    } else if (dx > -dy && dx > dy) {
        _sendCommand('right');
    } else if (dx < -dy && dx < dy) {
        _sendCommand('left');
    }
  }

  void stop() {
    _sendCommand('stop');
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<RobotController>(context, listen: false).checkInitialConnection();
    });
  }

  @override
  Widget build(BuildContext context) {
    final controller = Provider.of<RobotController>(context);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Robot Control'),
        actions: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16.0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Text('WiFi: ${controller.wifiStatus}'),
                Text('Status: ${controller.httpStatus}'),
              ],
            ),
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Text('Motors'),
                Joystick(
                  listener: (details) {
                    controller.move(details);
                  },
                  onStickDragEnd: (_) {
                    controller.stop();
                  }
                ),
              ],
            ),
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Text('Servo 1 (Up/Down)'),
                Joystick(
                  mode: JoystickMode.vertical,
                  listener: (details) {
                    controller.moveServo1(details);
                  },
                ),
              ],
            ),
            ElevatedButton(
              onPressed: () => controller.toggleServo2(),
              child: Text(
                  'Toggle Servo 2 (State: ${controller.servo2Toggled ? "180°" : "0°"})'),
            ),
          ],
        ),
      ),
    );
  }
}