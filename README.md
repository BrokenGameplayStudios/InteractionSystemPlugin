# Interaction System Plugin

A clean, fully replicated interaction system for Unreal Engine 5.

Add the component to your player and start interacting with the world. Includes built-in prompts, hold interactions, progress feedback, and notifications.

## Features

* Player `UInteractComponent` with line trace detection
* `UInteractableComponent` supporting Press and Hold interactions (with progress)
* Ready-to-use `ABaseInteractable` actor base class
* Built-in interaction prompt system (on-screen "Press E to...")
* `InteractionNotificationSubsystem` for temporary messages
* Hold progress tracking (replicated)
* Server-authoritative interactions
* Full multiplayer support
* Rich Blueprint delegates
* No dependencies

## Installation

1) Create folder `Plugins/InteractionSystemPlugin` in your project
2) Copy the plugin files from this repo into it
3) Enable the plugin in the Editor
4) Restart Editor

## Quick Start

1) Add **Interact Component** to your Character
2) In your Character Blueprint, on **PossessedBy**, call `Initialize` on the component
3) Add **Interactable Component** to any Actor you want to make interactive (or inherit from `BaseInteractable`)
4) Bind your "Interact" input action → call `TryInteract()` on the Interact Component
5) Customize interaction text, hold time, and visuals directly in the Interactable Component

## Delegates (bind in Character or Widgets)

* **OnFocusedInteractableChanged** → Update prompt UI
* **OnInteractStart** → VFX, sounds, animations
* **OnInteractComplete** → Main interaction logic
* **OnInteractProgress** → Hold progress bar
* **OnInteractCancel** → Cancel feedback

## About

A fully replicated, plug-and-play interaction system designed for easy extension and multiplayer games.
