export default class UIManager {
  constructor(talkMachine) {
    this.talkMachine = talkMachine;
    this.initElements();
  }

  initElements() {
    // USB elements
    this.connectButton = document.querySelector('#connect');
    this.statusDisplay = document.querySelector('#status');

    // Voice elements
    this.speakitbutton = document.querySelector('#speakit');
    this.inputTxt = document.querySelector('#texttospeak');
    this.pitch = document.querySelector('#pitch');
    this.rate = document.querySelector('#rate');
    this.voiceSelect = document.querySelector('#voiceSelect');
    this.allvoicecheck = document.querySelector('#allvoices');
    this.codeDisplay = document.querySelector('#code-display');

    // Console elements
    this.consoleOutput = document.querySelector('#console-output');
    this.clearConsoleButton = document.querySelector('#clear-console');

    // LED elements
    this.ledContainer = document.querySelector('.led-container');

    // Button elements
    this.testButtons = document.querySelectorAll('.testbutton');
    this.testerButtons = document.querySelectorAll('.tester');
    this.restartButton = document.querySelector('#restartbutton');
  }

  init() {
    this.initVoiceUI();
    this.initConsoleUI();
    this.initLEDUI();
    this.initLEDControls();
    this.initButtonUI();
  }

  /* Voice UI */
  initVoiceUI() {
    this.setupVoiceSelection();
    this.setupVoiceEventListeners();
  }

  setupVoiceSelection() {
    // Bind and set up voice selection events
    this.populateVoiceList = this.populateVoiceList.bind(this);
    speechSynthesis.addEventListener('voiceschanged', this.populateVoiceList);
    this.allvoicecheck.addEventListener('change', this.populateVoiceList);
  }

  setupVoiceEventListeners() {
    // Set up speak button and parameter change listeners
    const updateCodeDisplay = () => {
      const selectedOption = this.voiceSelect.selectedOptions[0];
      const voiceIndex = selectedOption
        ? parseInt(selectedOption.textContent.split(' - ')[0])
        : 0;
      const pitch = parseFloat(this.pitch.value);
      const rate = parseFloat(this.rate.value);
      const text = 'text...';

      this.codeDisplay.textContent = `this.speechText("${text}", [${voiceIndex}, ${pitch}, ${rate}]);`;
    };

    this.speakitbutton.addEventListener('click', () => {
      const selectedOption = this.voiceSelect.selectedOptions[0];
      const voiceIndex = parseInt(selectedOption.textContent.split(' - ')[0]);
      this.talkMachine.speechText(this.inputTxt.value, [
        voiceIndex,
        parseFloat(this.pitch.value),
        parseFloat(this.rate.value),
      ]);
    });

    // Update code display when parameters change
    this.inputTxt.addEventListener('input', updateCodeDisplay);
    this.pitch.addEventListener('input', updateCodeDisplay);
    this.rate.addEventListener('input', updateCodeDisplay);
    this.voiceSelect.addEventListener('change', updateCodeDisplay);
  }

  populateVoiceList() {
    // Clear existing options
    while (this.voiceSelect.firstChild) {
      this.voiceSelect.removeChild(this.voiceSelect.firstChild);
    }

    const voices = speechSynthesis.getVoices();
    const showAllVoices = this.allvoicecheck.checked;
    const systemLanguage = navigator.language;

    // Find system default voice
    const defaultVoice = voices.find((voice) => voice.default) || voices[0];

    voices.forEach((voice, index) => {
      // Show voice if either:
      // 1. Show all voices is checked, or
      // 2. Voice matches system language, or
      // 3. Voice is the default voice
      if (
        showAllVoices ||
        voice.lang.includes(systemLanguage) ||
        voice === defaultVoice
      ) {
        const option = document.createElement('option');
        option.textContent = `${index} - ${voice.name} (${voice.lang})${
          voice.default ? ' — DEFAULT' : ''
        }`;
        option.setAttribute('data-lang', voice.lang);
        option.setAttribute('data-name', voice.name);
        this.voiceSelect.appendChild(option);
      }
    });

    // Select the default voice
    if (defaultVoice) {
      const defaultOption = Array.from(this.voiceSelect.options).find(
        (option) => option.getAttribute('data-name') === defaultVoice.name
      );
      if (defaultOption) {
        this.voiceSelect.value = defaultOption.value;
      }
    }
  }

  /* Console UI */
  initConsoleUI() {
    // Initialize console output
    this.talkMachine.fancyLogger.setConsoleOutput(this.consoleOutput);

    // Add clear console button handler
    this.clearConsoleButton.addEventListener('click', () => {
      this.talkMachine.fancyLogger.clearConsole();
    });
  }

  /* LED UI */
  initLEDUI() {
    if (!this.ledContainer) {
      console.error('LED container not found');
      return;
    }

    // Clear existing content
    this.ledContainer.innerHTML = '';

    // Generate LED elements
    for (let i = 0; i < this.talkMachine.maxLeds; i++) {
      const led = document.createElement('div');
      led.id = 'led' + i;
      led.className = 'led';
      led.textContent = i;
      this.ledContainer.appendChild(led);
    }

    // Initialize all LEDs to off (black)
    this.turnOffAllLeds();
  }

  setLedState(ledIndex, color, effect = 0) {
    if (ledIndex < 0 || ledIndex >= this.talkMachine.maxLeds) return;

    const led = document.getElementById(`led${ledIndex}`);
    if (!led) return;

    // Remove all existing colors and effects
    led.style = {};
    led.className = 'led';

    // Add color if specified
    // check if color is hex value or name
    if (!color) return;
    if (color.startsWith('#')) {
      led.style.background = color;
      led.style.boxShadow = '0 0 10px ' + color;
    } else {
      led.classList.add(color.toLowerCase());
    }

    // Add effect if specified
    switch (effect) {
      case 1: // blink
        led.classList.add('blink');
        break;
      case 2: // pulse
        led.classList.add('pulse');
        break;
      // case 0 or default: no effect
    }
  }

  turnOffAllLeds() {
    for (let i = 0; i < this.talkMachine.maxLeds; i++) {
      // reset style
      const led = document.getElementById(`led${i}`);
      if (!led) return;
      led.className = 'led';
      // reset color and effect
      this.setLedState(i, 'black', 0);
    }
  }

  initLEDControls() {
    const allOffBtn = document.querySelector('#allLedsOff');
    const allTestBtn = document.querySelector('#allLedsTest');

    allOffBtn.addEventListener('click', () => {
      this.talkMachine.ledsAllOff();
    });

    allTestBtn.addEventListener('click', () => {
      this.talkMachine.ledsAllChangeColor('red', 0);
    });
  }

  /* Button UI */
  initButtonUI() {
    this.initSimulationButtons();
    this.initTestButtons();
    this.initMachineControls();
  }

  initSimulationButtons() {
    // test buttons
    for (let i = 0; i < this.testButtons.length; i++) {
      this.testButtons[i].addEventListener('mousedown', (e) => {
        const t = e.target;
        const btn = t.id.substring(3, 4);
        this.talkMachine.handleButtonPressed(btn, true);
      });
      this.testButtons[i].addEventListener('mouseup', (e) => {
        const t = e.target;
        const btn = t.id.substring(3, 4);
        this.talkMachine.handleButtonReleased(btn, true);
      });

      // Also handle touch events for mobile
      this.testButtons[i].addEventListener('touchstart', (e) => {
        const t = e.target;
        const btn = t.id.substring(3, 4);
        this.talkMachine.handleButtonPressed(btn, true);
      });
      this.testButtons[i].addEventListener('touchend', (e) => {
        const t = e.target;
        const btn = t.id.substring(3, 4);
        this.talkMachine.handleButtonReleased(btn, true);
      });
    }
  }

  initTestButtons() {
    if (!this.testerButtons) return;

    for (let i = 0; i < this.testerButtons.length; i++) {
      this.testerButtons[i].addEventListener('click', () => {
        this.talkMachine.handleTesterButtons(i + 1);
      });
    }
  }

  initMachineControls() {
    if (this.restartButton) {
      this.restartButton.addEventListener(
        'click',
        this.talkMachine.handleRestartButton.bind(this.talkMachine)
      );
    } else {
      console.error('Restart button not found');
    }
  }

  updateRestartButtonText(isStarted) {
    if (this.restartButton) {
      this.restartButton.textContent = isStarted
        ? 'Stop Dialog-Machine'
        : 'Start Dialog-Machine';
    }
  }
}
