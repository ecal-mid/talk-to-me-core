import WebUsbManager from './WebUsbManager.js';
import FancyLogger from './FancyLogger.js';
import UIManager from './UIManager.js';

/**
 * Base class for handling speech synthesis, LED control, and USB communication
 * @class
 */
export default class TalkMachine {
  /**
   * Initialize TalkMachine with speech synthesis, LED control, and USB communication
   * @constructor
   * @public
   */
  constructor() {
    // Button press handling - initialize first to ensure it's available
    this.longPressDelay = 700; // milliseconds for long press
    this.pressStartTime = null;
    this.isPressed = false;

    this.version = '1.5.0';
    this.versionDisplay = document.querySelector('#version');
    this.maxLeds = 10;

    // Speech synthesis initialization
    this.speechSynth = window.speechSynthesis;
    this.speechVoices = [];
    this.speechIsSpeaking = false;

    // Initialize managers
    this.fancyLogger = new FancyLogger();
    this.ui = new UIManager(this);
    this.webUsbManager = new WebUsbManager(
      this,
      this.ui.connectButton,
      this.ui.statusDisplay
    );
    this.webUsbManager.setLogger(this.fancyLogger);

    // USB command handling
    this.handleReceiveUSBCommand = this.receiveCommandFromUsb.bind(this);
    this.machineStarted = false;

    // Display version
    this.versionDisplay.textContent = 'version ' + this.version;

    // Initialize everything
    this.initLedColors();
    this.ui.init();
    this.speechInit();
  }

  /* SPEECH SYNTHESIS */
  /**
   * Initialize speech synthesis system
   * @private
   */
  speechInit() {
    this.speechInitVoices().then(() => {
      this.speechOnReady();
    });
  }

  /**
   * Initialize speech synthesis voices
   * @private
   * @returns {Promise} Resolves when voices are loaded
   */
  speechInitVoices() {
    return new Promise((resolve, reject) => {
      let id = setInterval(() => {
        if (this.speechSynth.getVoices().length !== 0) {
          resolve(this.speechSynth.getVoices());
          clearInterval(id);
        }
      }, 10);
    });
  }

  /**
   * Handle speech synthesis ready event
   * @private
   */
  speechOnReady() {
    this.speechVoices = this.speechSynth.getVoices();

    window.dispatchEvent(
      new CustomEvent('TextToSpeechReady', {
        detail: {},
      })
    );
  }

  /**
   * Speak the provided text with given options
   * @public
   * @param {string} text - Text to be spoken
   * @param {Array} options - [voiceIndex, pitch, rate]
   */
  speechText(text, options = [0, 1, 1]) {
    const voice = options[0];
    const pitch = options[1];
    const rate = options[2];

    // Cancel any ongoing speech
    this.speechCancel();

    this.speechUtterance = new SpeechSynthesisUtterance(text);
    this.speechUtterance.voice = this.speechVoices[voice];
    this.speechUtterance.pitch = pitch;
    this.speechUtterance.rate = rate;

    // Bind event handlers
    const boundFinishSpeaking = this.speechOnEnd.bind(this);
    this.speechUtterance.addEventListener('end', boundFinishSpeaking);
    this.speechUtterance.addEventListener('error', boundFinishSpeaking);

    this.speechIsSpeaking = true;
    this.speechSynth.speak(this.speechUtterance);
    this.fancyLogger.logSpeech(text);
  }

  /**
   * Handle speech end event
   * @private
   */
  speechOnEnd() {
    if (!this.speechIsSpeaking) return;
    this.speechIsSpeaking = false;

    // Call protected method that can be overridden by child classes
    this._handleTextToSpeechEnded();
  }

  /**
   * Handle text-to-speech end event
   * Can be overridden by child classes for custom behavior
   * @protected
   */
  _handleTextToSpeechEnded() {}

  /**
   * Cancel current speech
   * @public
   */
  speechCancel() {
    this.speechSynth.cancel();
    this.speechIsSpeaking = false;
  }

  /**
   * Get available speech synthesis voices
   * @public
   * @returns {Array} List of available voices
   */
  speechGetVoices() {
    return this.speechVoices;
  }

  /**
   * Handle incoming USB commands
   * @private
   * @param {string} data - Command data from USB
   */
  receiveCommandFromUsb(data) {
    const command = data.substring(0, 1);
    let val = data.substring(1);
    val = val.replace(/[\n\r]+/g, '');

    if (command == 'M') {
      this.fancyLogger.logMessage(val);
    }
    if (command == 'B') {
      // Button pressed from USB
      this.handleButtonPressed(val, false);
    }
    if (command == 'H') {
      // Button released from USB
      this.handleButtonReleased(val, false);
    }
    if (command == 'P') {
      dispatchPotentiometer(val);
    }
  }

  /**
   * Send command to USB device
   * @private
   * @param {string} data - Command to send
   */
  sendCommandToUsb(data) {
    this.webUsbManager.handleSend(data);
  }

  /* BUTTONS */
  /**
   * Handle tester button clicks - to be overridden by child class
   * @protected
   * @param {number} btn - Button number
   */
  _handleTesterButtons(btn) {}

  handleTesterButtons(btn) {
    // Call child class implementation
    this._handleTesterButtons(btn);
  }
  /**
   * Handle button press
   * @public
   * @param {number} button - Button number
   */
  handleButtonPressed(button, simulated = false) {
    // Record press start time and state
    this.pressStartTime = Date.now();
    this.isPressed = true;
    this.fancyLogger.logButton(
      button + ' pressed' + (simulated ? ' (simulated)' : '')
    );

    // Call child class implementation
    this._handleButtonPressed(button, simulated);
  }

  /**
   * Handle button release
   * @public
   * @param {number} button - Button number
   */
  handleButtonReleased(button, simulated = false) {
    // Only process if we have a valid press start time
    if (!this.pressStartTime || !this.isPressed) {
      return;
    }

    const pressDuration = Date.now() - this.pressStartTime;

    // Reset press state
    this.isPressed = false;

    if (pressDuration >= this.longPressDelay) {
      // Long press detected
      this.fancyLogger.logButton(
        button + ' longpress' + (simulated ? ' (simulated)' : '')
      );
      this._handleButtonLongPressed(button, simulated);
    } else {
      // Normal press released
      this.fancyLogger.logButton(
        button + ' released' + (simulated ? ' (simulated)' : '')
      );
      this._handleButtonReleased(button, simulated);
    }

    // Reset press start time after handling the event
    this.pressStartTime = null;
  }

  /**
   * Handle button press - to be overridden by child class
   * @param {number} button - Button number
   * @protected
   */
  _handleButtonPressed(button, simulated = false) {
    // Override in child class
  }

  /**
   * Handle button release - to be overridden by child class
   * @param {number} button - Button number
   * @protected
   */
  _handleButtonReleased(button, simulated = false) {
    // Override in child class
  }

  /**
   * Handle long button press - to be overridden by child class
   * @param {number} button - Button number
   * @protected
   */
  _handleButtonLongPressed(button, simulated = false) {
    // Override in child class
  }

  /**
   * Dispatch button events
   * @private
   * @param {string} val - Button value
   * @param {string} btn_state - Button state ('pressed', 'released', or 'longpress')
   * @param {boolean} simulated - Whether the event is simulated
   */
  dispatchButton(val, btn_state, simulated = false) {
    this.fancyLogger.logButton(
      val + ' ' + btn_state + (simulated ? ' (simulated)' : '')
    );
  }

  /* LEDS */
  /**
   * Initialize LED color mappings
   * @private
   */
  initLedColors() {
    this.colorLeds = {
      black: '00',
      white: '01',
      red: '02',
      green: '03',
      blue: '04',
      magenta: '05',
      yellow: '06',
      cyan: '07',
      orange: '08',
      purple: '09',
      pink: '10',
    };
  }

  /**
   * Change color of a specific LED
   * @public
   * @param {number} led_index - Index of the LED
   * @param {string} led_color - Color name from: black, white, red, green, blue, magenta, yellow, cyan, orange, purple, pink
   * @param {number} led_effect - Effect number (0: static, 1: blink, 2: pulse, 3: vibrate)
   */
  ledChangeColor(led_index, led_color, led_effect = 0) {
    const led_color_code = this.colorLeds[led_color];
    this.sendCommandToUsb('L' + led_index + led_color_code + led_effect);
    this.ui.setLedState(led_index, led_color, led_effect);
  }

  /**
   * Change color of all LEDs
   * @public
   * @param {string} led_color - Color name from: black, white, red, green, blue, magenta, yellow, cyan, orange, purple, pink
   * @param {number} led_effect - Effect number (0: static, 1: blink, 2: pulse, 3: vibrate)
   */
  ledsAllChangeColor(led_color, led_effect = 0) {
    this.sendCommandToUsb('Lx000');
    for (let i = 0; i < this.maxLeds; i++) {
      const led_color_code = this.colorLeds[led_color];
      this.sendCommandToUsb('L' + i + led_color_code + led_effect);
      this.ui.setLedState(i, led_color, led_effect);
    }
  }

  /**
   * Turn off all LEDs
   * @public
   */
  ledsAllOff() {
    this.sendCommandToUsb('Lx000');
    this.ui.turnOffAllLeds();
  }

  /**
   * Change LED color using RGB values
   * @public
   * @param {number} led_index - Index of the LED
   * @param {number} r - Red value (0-255)
   * @param {number} g - Green value (0-255)
   * @param {number} b - Blue value (0-255)
   */
  ledChangeRGB(led_index = 0, r = 255, g = 255, b = 255) {
    let led_index_zeroed;
    if (led_index < 10) {
      led_index_zeroed = '0' + led_index;
    } else {
      led_index_zeroed = led_index;
    }
    r = r.toString(16);
    g = g.toString(16);
    b = b.toString(16);

    if (r.length == 1) r = '0' + r;
    if (g.length == 1) g = '0' + g;
    if (b.length == 1) b = '0' + b;

    const hex_color = r + g + b;
    this.sendCommandToUsb('H' + led_index_zeroed + hex_color);
    this.ui.setLedState(led_index, '#' + hex_color, 0);
  }

  /**
   * Change all LEDs using RGB values
   * @public
   * @param {number} r - Red value (0-255)
   * @param {number} g - Green value (0-255)
   * @param {number} b - Blue value (0-255)
   */
  ledsAllChangeRGB(r = 255, g = 255, b = 255) {
    r = r.toString(16);
    g = g.toString(16);
    b = b.toString(16);

    if (r.length == 1) r = '0' + r;
    if (g.length == 1) g = '0' + g;
    if (b.length == 1) b = '0' + b;

    const hex_color = r + g + b;
    for (let i = 0; i < this.maxLeds; i++) {
      let i_zeroed;
      if (i < 10) {
        i_zeroed = '0' + i;
      } else {
        i_zeroed = i;
      }
      this.sendCommandToUsb('H' + i_zeroed + hex_color);
      this.ui.setLedState(i, '#' + hex_color, 0);
    }
  }

  /* MACHINE CONTROL */
  /**
   * Handle restart button click
   * @public
   */
  handleRestartButton() {
    if (!this.dialogStarted) {
      this.ui.updateRestartButtonText(true);
      this.start();
    } else {
      this.ui.updateRestartButtonText(false);
      this.stop();
    }
  }

  /**
   * Start dialog - to be overridden by child class
   * @public
   */
  startDialog() {}

  /**
   * Start the machine
   * @public
   */
  start() {
    this.dialogStarted = true;
    this.startDialog();
  }

  /**
   * Stop the machine
   * @public
   */
  stop() {
    this.dialogStarted = false;
    this.ledsAllOff();
    this.fancyLogger.clearConsole();
  }
}
