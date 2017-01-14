module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "toggle",
        "messageKey": "Animation",
        "label": "Enable Blinking Animation",
        "defaultValue": true
      },
			{
        "type": "toggle",
        "messageKey": "Vibration",
        "label": "Vibrate on Disconnect",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];