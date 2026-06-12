# Quest System UI Concept Documentation

## Visual Design Philosophy
The quest system UI should reflect the primitive, stone-age setting of the game while remaining functional and readable for modern players.

## Resource Gathering Quest Interface

### Primary Elements
- **Quest Tracker Panel**: Semi-transparent overlay showing active objectives
- **Resource Icons**: Simple, recognizable symbols for each resource type
  - Stone: Gray rock silhouette
  - Wood: Brown branch/log shape
  - Plant: Green leaf cluster
  - Water: Blue droplet
  - Bone: White curved bone shape
  - Hide: Brown animal skin texture

### Progress Display
- **Format**: "Resource Name: Current/Required"
- **Example**: "Gather Stones: 3/5"
- **Progress Bar**: Stone-carved appearance with filled sections

### Color Scheme
- **Background**: Dark brown leather texture
- **Text**: Cream/bone white for readability
- **Highlights**: Amber/orange for completed objectives
- **Accent**: Earth tones (ochre, sienna, umber)

### Typography
- **Style**: Bold, carved appearance suggesting primitive inscriptions
- **Size**: Large enough for clear reading during gameplay
- **Contrast**: High contrast against background for accessibility

### Interactive Elements
- **Hover States**: Subtle glow effect suggesting firelight
- **Click Feedback**: Stone-tap sound effect
- **Animation**: Smooth fade transitions, no jarring effects

### Positioning
- **Location**: Upper left corner of screen
- **Size**: Compact but readable (approximately 300x200 pixels)
- **Transparency**: 80% opacity to avoid blocking gameplay view

## Discovery Quest Interface

### Exploration Markers
- **Map Pins**: Bone spike markers for discovered locations
- **Territory Boundaries**: Rough-drawn lines suggesting hand-carved maps
- **Discovery Counter**: "Territories Explored: X/Y"

### Visual Feedback
- **New Discovery**: Brief amber flash effect
- **Completed Area**: Filled territory with subtle texture
- **Unexplored**: Darker, shadowed regions

## Technical Implementation Notes
- Use UMG (Unreal Motion Graphics) for UI creation
- Implement responsive scaling for different screen resolutions
- Ensure compatibility with controller and keyboard/mouse input
- Include accessibility options for colorblind players

## Audio Integration
- **Quest Start**: Deep drum beat or horn sound
- **Progress Update**: Subtle stone-tap or wood-knock sound
- **Quest Complete**: Triumphant but primitive horn fanfare
- **Resource Collected**: Appropriate material sound (stone clink, wood thud, etc.)

## Animation Principles
- **Entrance**: Slide in from left edge
- **Updates**: Gentle pulse on progress change
- **Exit**: Fade out over 2 seconds
- **Transitions**: Smooth easing, no sharp movements

This UI concept maintains immersion in the prehistoric setting while providing clear, functional quest guidance for players.