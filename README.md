# 🎮 Game Design with Microprocessor Systems

---

## 📘 Project Overview
This project was developed as part of the **Microprocessor Systems and Interfacing (Purdue ECE 362)** course. The objective was to design and implement a mini game using an embedded system while applying key concepts such as external interface formats and internal peripheral subsystems.

Using the **STM32F091 microcontroller**, we created a rhythm-based game inspired by **Dance Dance Revolution** and **Piano Tiles**.

---

## 👥 Team Members
- Esharaqa Jahid
- Yashvi Agrawal
- Hilal Beyza Taşdemir  
- Sitara Iyer  

---

## 🎵 Game Description
Directional arrows fall down the screen in synchronization with a song’s beat. The player interacts with the system using a keypad and must press the corresponding arrow key as it reaches the bottom of the display. The score increases or decreases accordingly based on input accuracy, while LED indicators provide immediate visual feedback for correct and incorrect inputs.

---

## 🧩 System Features

### 🔹 Hardware Components
- **TFT LCD Display**  
  Renders animations, falling arrows, and overall game visuals.

- **Keypad**  
  Captures user input for directional commands.

- **Seven-Segment Displays**  
  Displays the player’s current score dynamically.

- **LED Indicators**  
  - 🟢 Green LED: Correct input  
  - 🔴 Red LED: Incorrect input  

---

## ⚙️ Internal Design Highlights
- **GPIO** for input/output management  
- **Timers** to synchronize gameplay with the music  
- **DAC** for audio signal generation  
- **Interrupts** for real-time responsiveness  

---

## 🧠 Learning Outcomes
This project strengthened our understanding of:
- Embedded systems design  
- Microcontroller peripheral integration  
- Real-time system implementation  
- Debugging and problem-solving  
- Team collaboration and project coordination  

---

## 🚀 Conclusion
This project was an excellent opportunity to apply theoretical knowledge in a hands-on environment. We are proud of the final system and grateful for the collaboration throughout the development process.

We look forward to tackling more challenges in **embedded systems** and **microcontroller-based applications**. A huge thank you to our Professor Younghyun Kim and Lab Coordinator Niraj Menon for their guidance and support throughout this course.
