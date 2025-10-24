# 🎮 Game Controller to MIDI for Mixing Software 🎹

This document explains how to use a game controller in place of a traditional mixing table for software like **VirtualDJ**, using the provided C++ code.

The code was tested with a **DualShock 4** controller but should be compatible with any **Windows-compatible controller**.

***

## 🕹️ Controller Usage

The controller provides access to the virtualDJ buttons and sliders, provided you have mapped the controls according to the joined text file.
> you can use the mappings in the joined XML file, just copy/paste it in the  `C:\users\YOURNAME\Documents\VirtualDJ\Mappers` folder

### 🎚️ Joystick Functionality

The core idea is to use the **vertical axis of both joysticks** to control various sliders and knobs.

* By default, they are bound to the **volume sliders**:
    * **Left joystick** $\rightarrow$ **Left Deck** volume.
    * **Right joystick** $\rightarrow$ **Right Deck** volume.

> **What is a "Deck"?**
> A deck is a set of controls for a single music track. In typical mixing software, you have two tracks, hence two decks: one on the left and one on the right.

### 🔄 Operating Modes

The controller operates in two primary modes:

1.  **Default Mode:**
    * Each button press translates directly to a button action on a deck.
    * Use **DPAD\_LEFT** or **DPAD\_RIGHT** (controller left/right arrows) to select the **active deck** for button actions.

2.  **Binding Mode:**
    * The controller enters this mode when the **LEFT\_TOP\_TRIGGER** or **RIGHT\_TOP\_TRIGGER** (also known as controller shoulders) is pressed.
    * The **next button you press** while in binding mode will determine which slider/knob the associated joystick's vertical axis will be assigned to.
        * **LEFT\_TOP\_TRIGGER** defines the binding for the **left joystick**.
        * **RIGHT\_TOP\_TRIGGER** defines the binding for the **right joystick**.

***

## 🎯 Advanced Deck Assignment

What if you need the right joystick to adjust a knob on the left deck?

**Fear Not, My Child !!!** When you enter **Binding Mode** for the **right joystick** (by pressing **RIGHT\_TOP\_TRIGGER**), you can use **DPAD\_LEFT** or **DPAD\_RIGHT** (before choosing the button you want to bind) to choose which deck the right joystick will be assigned to. This allows both joysticks to control elements of a single deck.

***

## 📝 Important Notes for Customization

### NOTE 1: Dual Binding
You **can bind both joysticks** to the **exact same slider/knob**. This will work without issue.

### NOTE 2: Unbound Buttons
Not all buttons refer to a control axis in binding mode. If you press an **unbound button** while in binding mode, the joystick will **not do anything until you bind it again** to a valid control axis.

### NOTE 3: Editing Bindings
Feel free to edit the bindings mapping to suit your needs. You'll need to modify two variables in the code:

* `controllerAxisToButtonBind`: What controller axis is assigned to what button (what button was last pressed in binding mode)
    * Vertical joystick axes are index **1** (left) and **3** (right).
    * Use **-1** if an axis is not being used.
* `buttonToAxisBindings`: Specifies which custom axis should be updated when a joystick is bound to a specific button.
    * You can find the indexes for buttons and actions in `src/buttonsName.h` and `src/actionsName.h`.
    * Use **-1** if an button is not being used.

### NOTE 4: Controller Button Indices
If you modify the C++ code, be aware that the **controller button indices** in the code are **offset by 1** from the button numbers displayed in the Windows Game Controller window (e.g., index **5** in the code is displayed as **Button 6** in the Windows utility).