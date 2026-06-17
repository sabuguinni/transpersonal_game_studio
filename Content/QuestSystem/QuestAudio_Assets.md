# Quest System Audio Assets

## Generated Voice Narrations

### QuestGiver_Hunt.mp3
**Character**: Tribal Elder  
**Text**: "New quest available: Hunt the Alpha Predator. A massive Tyrannosaurus Rex has been terrorizing the northern territories. Track it down and eliminate the threat to secure safe passage for your tribe."  
**Usage**: Plays when player enters QuestTrigger_HuntZone_001  
**Duration**: ~15 seconds  
**Emotion**: Serious, urgent, authoritative  

### QuestGiver_Escort.mp3
**Character**: Scout  
**Text**: "Urgent mission: Escort survivors to the evacuation point. Several tribe members are trapped near the combat zone. Guide them safely along the marked evacuation routes before the predators return."  
**Usage**: Plays when player enters QuestTrigger_EscortZone_001  
**Duration**: ~13 seconds  
**Emotion**: Urgent, concerned, desperate  

## Sound Effects Needed (from search_sounds)

### Quest Complete Sounds
- Tribal drums fanfare
- Achievement notification
- Success chime
- Reward jingle

### Quest Start Sounds
- Mission begin notification
- Objective marker ping
- Quest accepted confirmation
- Map marker placement sound

## Implementation Notes

### Audio Triggers
- Quest giver voice lines play once per quest activation
- Quest complete sounds play on objective completion
- Quest start sounds play on trigger volume entry
- Audio ducking applied during combat to prevent overlap

### Audio System Integration (Agent #16)
- All quest audio routed through MetaSounds system
- 3D spatial audio for quest markers (audible within 50m)
- 2D UI audio for quest notifications
- Volume settings: Quest VO at 80%, SFX at 60%

### Localization Support
- All voice lines recorded in English (default)
- Text subtitles required for accessibility
- Future: Portuguese, Spanish, French localizations
