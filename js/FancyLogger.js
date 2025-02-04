export default class FancyLogger {
  constructor() {
    this.consoleOutput = null;
    this.logToNativeConsole = false;
    this.init();
  }

  init() {
    // STYLES
    this.LOGSTYLE_IN = [
      'color: #444',
      'background-color: pink',
      'padding: 2px 4px',
      'border-radius: 4px',
      'font-size: 1.3em;',
    ].join(';');

    this.LOGSTYLE_OUT = [
      'color: #444',
      'background-color: lightblue',
      'padding: 2px 4px',
      'border-radius: 4px',
      'font-size: 1.3em;',
    ].join(';');

    this.LOGSTYLE_WARN = [
      'color: #444',
      'background-color: lightyellow',
      'padding: 2px 4px',
      'border-radius: 4px',
      'font-size: 1.3em;',
    ].join(';');

    this.LOGSTYLE_MESSAGE = [
      'color: #444',
      'background-color: lightgray',
      'padding: 2px 4px',
      'border-radius: 4px',
      'font-size: 1.3em;',
    ].join(';');

    this.LOGSTYLE_CODE = [
      'color: #444',
      'background-color: lightgray',
      'padding: 8px 8px',
      'border-radius: 1px',
      'font-size: 1.3em;',
    ].join(';');

    this.LOGSTYLE_EMOJI = ['font-size: 1.5em;', 'padding: 2px 4px'].join(';');
    // https://www.w3schools.com/charsets/ref_emoji.asp
    this.EMO_BTN = String.fromCodePoint(0x26aa);
    this.EMO_MESSAGE = String.fromCodePoint(0x1f4ac);
    this.EMAO_SOUND = String.fromCodePoint(0x1f3b5);
    this.EMO_SPEECH = String.fromCodePoint(0x1f444);
    this.EMO_LED = String.fromCodePoint(0x1f6a5);
    this.EMO_WARNING = String.fromCodePoint(0x1f590);
    this.EMO_STATE = String.fromCodePoint(0x23ec);
    this.EMO_CODE = String.fromCodePoint(0x1f4c4);
  }

  setConsoleOutput(element) {
    this.consoleOutput = element;
  }

  clearConsole() {
    if (this.consoleOutput) {
      this.consoleOutput.innerHTML = '';
      this.consoleOutput.scrollTop = 0;
    }
    console.clear();
  }

  appendToConsole(message, type, emoji) {
    if (!this.consoleOutput) return;

    const messageDiv = document.createElement('div');
    messageDiv.className = `console-message ${type}`;

    const emojiSpan = document.createElement('span');
    emojiSpan.className = 'console-emoji';
    emojiSpan.textContent = emoji;

    messageDiv.appendChild(emojiSpan);
    messageDiv.appendChild(document.createTextNode(message));

    this.consoleOutput.appendChild(messageDiv);
    this.consoleOutput.scrollTop = this.consoleOutput.scrollHeight;
  }

  logMessage(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_MESSAGE} ${_msg}`, this.LOGSTYLE_IN);
    }
    this.appendToConsole(_msg, 'in', this.EMO_MESSAGE);
  }

  logWarning(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_WARNING} ${_msg}`, this.LOGSTYLE_WARN);
    }
    this.appendToConsole(_msg, 'warn', this.EMO_WARNING);
  }

  logButton(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_BTN} ${_msg}`, this.LOGSTYLE_IN);
    }
    this.appendToConsole(_msg, 'in', this.EMO_BTN);
  }

  logCode(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_CODE} ${_msg}`, this.LOGSTYLE_CODE);
    }
    this.appendToConsole(_msg, 'code', this.EMO_CODE);
  }

  logSound(_msg = '') {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMAO_SOUND} ${_msg}`, this.LOGSTYLE_OUT);
    }
    this.appendToConsole(_msg, 'out', this.EMAO_SOUND);
  }

  logSpeech(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_SPEECH} ${_msg}`, this.LOGSTYLE_OUT);
    }
    this.appendToConsole(_msg, 'out', this.EMO_SPEECH);
  }

  logState(_msg) {
    if (this.logToNativeConsole) {
      console.log(`%c${this.EMO_STATE} ${_msg}`, this.LOGSTYLE_MESSAGE);
    }
    this.appendToConsole(_msg, 'message', this.EMO_STATE);
  }

  logLed(led_index, led_color, led_effect) {
    let LOGSTYLE_LED_COLOR_CUSTOM =
      'color: #444;background-color: ' +
      led_color +
      ';padding: 2px 5px;border-radius: 10px;font-size: 1.3em;';
    if (led_effect == 0) led_effect = 'no effect';
    if (led_effect == 1) led_effect = led_effect + ' blink';
    if (led_effect == 2) led_effect = led_effect + ' pulse';
    if (led_effect == 3) led_effect = led_effect + ' vibrate';

    const msg = `index: ${led_index}, color: ${led_color}, effect: ${led_effect}`;
    if (this.logToNativeConsole) {
      console.log(
        `%c${this.EMO_LED} ${msg}%c `,
        this.LOGSTYLE_OUT,
        LOGSTYLE_LED_COLOR_CUSTOM
      );
    }
    this.appendToConsole(msg, 'out', this.EMO_LED);
  }
}
