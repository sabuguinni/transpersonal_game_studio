#include "GameBible.h"
#include "Engine/Engine.h"

UGameBible::UGameBible()
{
    // Initialize core narrative elements
    GameTitle = FText::FromString("Transpersonal: Echoes of the Mesozoic");
    
    CorePremise = FText::FromString(
        "A paleontologist, displaced through time by a mysterious crystal, must survive in the Mesozoic era "
        "while searching for another crystal to return home. But in this journey, they discover that survival "
        "is not just about staying alive—it's about understanding their place in the grand tapestry of life."
    );
    
    CentralTheme = FText::FromString(
        "The interconnectedness of all life across time, and how understanding our place in nature's "
        "grand design can transform fear into wonder, isolation into connection."
    );
    
    EmotionalCore = FText::FromString(
        "From the terror of being prey to the awe of witnessing life in its purest form, "
        "the protagonist's journey is one of emotional and spiritual transformation."
    );

    // Primary themes
    PrimaryThemes = {
        ENarrativeTheme::Survival,
        ENarrativeTheme::Discovery,
        ENarrativeTheme::Wonder,
        ENarrativeTheme::TimeDisplacement,
        ENarrativeTheme::Connection
    };

    // Initialize protagonist profile
    ProtagonistProfile.CharacterID = FName("DrProtagonist");
    ProtagonistProfile.Archetype = ECharacterArchetype::ScientistProtagonist;
    ProtagonistProfile.CharacterName = FText::FromString("Dr. [Player Name]");
    
    ProtagonistProfile.BackgroundStory = FText::FromString(
        "A dedicated paleontologist who has spent their life studying ancient life through fossils. "
        "Methodical, curious, and deeply respectful of the scientific method. Has always wondered "
        "what it would be like to see these creatures alive, but never imagined the terror and "
        "wonder that would come with actually experiencing it."
    );
    
    ProtagonistProfile.PersonalityTraits = FText::FromString(
        "Analytical mind, deep curiosity, initial fear that transforms into wonder, "
        "strong survival instinct, growing empathy for prehistoric life, "
        "internal conflict between scientific objectivity and emotional connection."
    );
    
    ProtagonistProfile.NarrativeRole = FText::FromString(
        "The bridge between modern understanding and ancient wisdom. "
        "Represents humanity's relationship with nature—from exploitation to understanding to harmony."
    );

    ProtagonistProfile.RelatedThemes = {
        ENarrativeTheme::Discovery,
        ENarrativeTheme::Adaptation,
        ENarrativeTheme::TimeDisplacement
    };

    ProtagonistProfile.CharacterArcs = {
        "From Terror to Wonder: Initial fear of dinosaurs transforms into awe and respect",
        "From Observer to Participant: Shifts from studying life to living as part of it",
        "From Isolation to Connection: Learns to communicate and coexist with prehistoric life",
        "From Survival to Harmony: Evolves from mere survival to finding balance in the ecosystem"
    };

    // Character growth milestones
    CharacterGrowthMilestones = {
        "First Encounter: The moment of arrival and initial terror",
        "First Survival Night: Learning to hide and stay alive",
        "First Tool Creation: Beginning to adapt and problem-solve",
        "First Dinosaur Observation: Shifting from fear to scientific curiosity",
        "First Peaceful Encounter: Realizing not all dinosaurs are threats",
        "First Domestication Attempt: Trying to connect with prehistoric life",
        "First Successful Bond: Achieving trust with a dinosaur companion",
        "Understanding the Ecosystem: Seeing the bigger picture of life's web",
        "Finding the Second Crystal: The choice between return and staying",
        "The Final Decision: Choosing what 'home' truly means"
    };

    // World description
    WorldDescription = FText::FromString(
        "A lush, primordial world where every breath carries the scent of ancient ferns and "
        "the distant calls of creatures that shouldn't exist. This is Earth as it was millions "
        "of years ago—untamed, magnificent, and utterly alien to modern human experience. "
        "Every shadow could hide a predator, every sound could signal danger, yet every "
        "sunrise reveals beauty beyond imagination."
    );

    TimeAndPlace = FText::FromString(
        "Late Jurassic to Early Cretaceous period, approximately 150-100 million years ago. "
        "A temperate region with dense forests, open plains, river systems, and coastal areas. "
        "The climate is warm and humid, supporting incredible biodiversity."
    );

    // Act descriptions
    ActDescriptions.Add(EStoryAct::Act1_Arrival, FText::FromString(
        "THE DISPLACEMENT: The protagonist discovers the crystal and is transported. "
        "Initial shock, terror, and desperate attempts at survival. Establishing the rules "
        "of this world and the protagonist's vulnerability."
    ));
    
    ActDescriptions.Add(EStoryAct::Act2_Adaptation, FText::FromString(
        "LEARNING TO SURVIVE: The protagonist begins to understand this world. "
        "Building shelter, finding food, avoiding predators. First attempts at "
        "tool-making and basic survival skills."
    ));
    
    ActDescriptions.Add(EStoryAct::Act3_Understanding, FText::FromString(
        "SEEING THE PATTERNS: Scientific curiosity begins to overcome fear. "
        "Observing dinosaur behavior, understanding ecosystems, first attempts "
        "at peaceful interaction with prehistoric life."
    ));
    
    ActDescriptions.Add(EStoryAct::Act4_Integration, FText::FromString(
        "BECOMING PART OF THE WORLD: Successfully domesticating or befriending "
        "dinosaurs. Understanding one's place in the ecosystem. The search for "
        "the return crystal becomes more complex as attachment to this world grows."
    ));
    
    ActDescriptions.Add(EStoryAct::Act5_Departure, FText::FromString(
        "THE CHOICE: Finding the second crystal and facing the ultimate decision. "
        "Return to the modern world or stay in this prehistoric paradise? "
        "What does 'home' really mean?"
    ));

    ClimaxDescription = FText::FromString(
        "Standing before the second crystal, the protagonist must choose between "
        "returning to their old life or embracing their new understanding of "
        "humanity's place in nature. This choice is complicated by bonds formed "
        "with dinosaur companions and a deep spiritual connection to this ancient world."
    );

    // Possible endings
    PossibleEndings = {
        FText::FromString("The Return: Goes back to modern times but forever changed, "
                         "becoming an advocate for environmental protection and species preservation."),
        FText::FromString("The New Life: Chooses to stay, becoming the bridge between "
                         "human consciousness and prehistoric wisdom."),
        FText::FromString("The Synthesis: Finds a way to bring elements of both worlds together, "
                         "perhaps bringing a dinosaur companion to the modern world or "
                         "establishing a connection between times.")
    };

    // Narrative constraints
    NarrativeConstraints = {
        "No anachronistic technology beyond what a paleontologist might carry",
        "Dinosaur behavior must be scientifically plausible, not movie monsters",
        "The protagonist cannot become superhuman - remains vulnerable throughout",
        "No other humans in the prehistoric world - complete isolation initially",
        "The crystal's power is mysterious but not magical - has scientific basis",
        "Death is always a real possibility - maintain tension and stakes",
        "Environmental storytelling is primary - minimal exposition dialogue"
    };

    // Tone guidelines
    ToneGuidelines = {
        "Awe and Wonder: Every encounter with prehistoric life should evoke amazement",
        "Constant Tension: Safety is never guaranteed, vigilance is always required",
        "Scientific Curiosity: Maintain the protagonist's analytical nature",
        "Emotional Authenticity: Fear, loneliness, joy, and connection must feel real",
        "Respect for Nature: Dinosaurs are not monsters but complex living beings",
        "Spiritual Undertones: Hint at deeper connections between all life",
        "Bittersweet Beauty: The world is magnificent but the protagonist doesn't belong"
    };

    // Dialogue guidelines
    DialogueGuidelines = {
        "Internal Monologue Primary: Most dialogue is the protagonist's thoughts",
        "Scientific Observations: Dialogue often includes analytical observations",
        "Emotional Honesty: The protagonist admits fear, wonder, loneliness",
        "Minimal Exposition: Let the world and actions tell the story",
        "Dinosaur 'Communication': Non-verbal, behavioral, empathetic understanding",
        "Progressive Confidence: Dialogue becomes more assured as skills develop",
        "Philosophical Moments: Quiet reflections on life, time, and belonging"
    };
}

FStoryBeat UGameBible::GetStoryBeat(FName BeatID) const
{
    for (const FStoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            return Beat;
        }
    }
    
    // Return empty beat if not found
    return FStoryBeat();
}

TArray<FStoryBeat> UGameBible::GetStoryBeatsForAct(EStoryAct Act) const
{
    TArray<FStoryBeat> ActBeats;
    
    for (const FStoryBeat& Beat : StoryBeats)
    {
        if (Beat.Act == Act)
        {
            ActBeats.Add(Beat);
        }
    }
    
    return ActBeats;
}

FCharacterNarrativeProfile UGameBible::GetCharacterProfile(FName CharacterID) const
{
    if (ProtagonistProfile.CharacterID == CharacterID)
    {
        return ProtagonistProfile;
    }
    
    for (const FCharacterNarrativeProfile& Profile : DinosaurCharacters)
    {
        if (Profile.CharacterID == CharacterID)
        {
            return Profile;
        }
    }
    
    // Return empty profile if not found
    return FCharacterNarrativeProfile();
}

FNarrativeLocation UGameBible::GetLocationNarrative(FName LocationID) const
{
    for (const FNarrativeLocation& Location : KeyLocations)
    {
        if (Location.LocationID == LocationID)
        {
            return Location;
        }
    }
    
    // Return empty location if not found
    return FNarrativeLocation();
}

TArray<ENarrativeTheme> UGameBible::GetActiveThemes(const FGameplayTagContainer& CurrentTags) const
{
    TArray<ENarrativeTheme> ActiveThemes;
    
    // Logic to determine active themes based on current game state
    // This would be expanded based on specific gameplay tags
    
    if (CurrentTags.HasTag(FGameplayTag::RequestGameplayTag("Narrative.Fear")))
    {
        ActiveThemes.AddUnique(ENarrativeTheme::Fear);
    }
    
    if (CurrentTags.HasTag(FGameplayTag::RequestGameplayTag("Narrative.Discovery")))
    {
        ActiveThemes.AddUnique(ENarrativeTheme::Discovery);
    }
    
    if (CurrentTags.HasTag(FGameplayTag::RequestGameplayTag("Narrative.Wonder")))
    {
        ActiveThemes.AddUnique(ENarrativeTheme::Wonder);
    }
    
    // Always include survival as it's core to the experience
    ActiveThemes.AddUnique(ENarrativeTheme::Survival);
    
    return ActiveThemes;
}