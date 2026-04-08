#include "CharacterArtistSystem.h"
#include "Engine/Engine.h"

UCharacterArtistSystem::UCharacterArtistSystem()
{
    MaxUniqueCharacters = 50;
    bEnsureVisualDiversity = true;
    MinimumVariationThreshold = 0.3f;
}

FCharacterDefinition UCharacterArtistSystem::CreateCharacter(ECharacterArchetype Archetype, ECharacterGender Gender)
{
    FCharacterDefinition NewCharacter;
    
    // Set basic identity
    NewCharacter.Archetype = Archetype;
    
    // Determine gender
    if (Gender == ECharacterGender::Random)
    {
        NewCharacter.Gender = FMath::RandBool() ? ECharacterGender::Male : ECharacterGender::Female;
    }
    else
    {
        NewCharacter.Gender = Gender;
    }
    
    // Determine age based on archetype
    switch (Archetype)
    {
        case ECharacterArchetype::TribalChild:
            NewCharacter.Age = ECharacterAge::Child;
            break;
        case ECharacterArchetype::TribalElder:
            NewCharacter.Age = ECharacterAge::Elder;
            break;
        case ECharacterArchetype::Protagonist:
            NewCharacter.Age = ECharacterAge::Adult; // Paleontologist is experienced adult
            break;
        default:
            // Random age for other archetypes, weighted towards adult
            float AgeRoll = FMath::RandRange(0.0f, 1.0f);
            if (AgeRoll < 0.1f) NewCharacter.Age = ECharacterAge::Young;
            else if (AgeRoll < 0.7f) NewCharacter.Age = ECharacterAge::Adult;
            else NewCharacter.Age = ECharacterAge::Elder;
            break;
    }
    
    // Generate physical variation
    NewCharacter.PhysicalVariation = GenerateVariationForArchetype(Archetype, NewCharacter.Age);
    
    // Generate name
    NewCharacter.CharacterName = GenerateCharacterName(NewCharacter.Gender, Archetype);
    
    // Generate personality traits
    NewCharacter.PersonalityTraits = GeneratePersonalityTraits(Archetype);
    
    // Generate background story
    NewCharacter.BackgroundStory = GenerateBackgroundStory(Archetype, NewCharacter.Gender);
    
    return NewCharacter;
}

FCharacterDefinition UCharacterArtistSystem::CreateProtagonist()
{
    FCharacterDefinition Protagonist = CreateCharacter(ECharacterArchetype::Protagonist, ECharacterGender::Male);
    
    // Specific customization for the protagonist
    Protagonist.CharacterName = "Dr. Marcus Thorne"; // Temporary name - will be set by Miguel
    Protagonist.PhysicalVariation.WearLevel = 0.1f; // Starts clean, will get dirty
    Protagonist.PhysicalVariation.ScarLevel = 0.0f; // No scars initially
    Protagonist.PhysicalVariation.DirtLevel = 0.1f; // Clean at start
    Protagonist.PhysicalVariation.BodyBuild = 0.4f; // Academic build, not muscular
    
    // Protagonist-specific traits
    Protagonist.PersonalityTraits = {
        "Curious and analytical",
        "Determined survivor",
        "Scientific mindset",
        "Adaptable under pressure",
        "Respectful of nature"
    };
    
    Protagonist.BackgroundStory = FText::FromString(
        "Dr. Marcus Thorne was conducting a routine geological survey when he discovered the mysterious glowing gem. "
        "A respected paleontologist with years of fieldwork experience, he now finds himself using every survival "
        "skill he's learned in the most dangerous environment imaginable - the age of dinosaurs."
    );
    
    return Protagonist;
}

TArray<FCharacterDefinition> UCharacterArtistSystem::CreateTribalCommunity(int32 CommunitySize)
{
    TArray<FCharacterDefinition> Community;
    
    // Ensure we have essential community members
    Community.Add(CreateCharacter(ECharacterArchetype::TribalElder, ECharacterGender::Random));
    Community.Add(CreateCharacter(ECharacterArchetype::TribalShaman, ECharacterGender::Random));
    Community.Add(CreateCharacter(ECharacterArchetype::TribalHunter, ECharacterGender::Male));
    Community.Add(CreateCharacter(ECharacterArchetype::TribalGatherer, ECharacterGender::Female));
    
    // Fill remaining slots with varied archetypes
    for (int32 i = 4; i < CommunitySize; ++i)
    {
        ECharacterArchetype RandomArchetype;
        float ArchetypeRoll = FMath::RandRange(0.0f, 1.0f);
        
        if (ArchetypeRoll < 0.3f) RandomArchetype = ECharacterArchetype::TribalHunter;
        else if (ArchetypeRoll < 0.6f) RandomArchetype = ECharacterArchetype::TribalGatherer;
        else if (ArchetypeRoll < 0.8f) RandomArchetype = ECharacterArchetype::TribalChild;
        else if (ArchetypeRoll < 0.9f) RandomArchetype = ECharacterArchetype::Survivor;
        else RandomArchetype = ECharacterArchetype::Wanderer;
        
        Community.Add(CreateCharacter(RandomArchetype, ECharacterGender::Random));
    }
    
    return Community;
}

FCharacterVariation UCharacterArtistSystem::GenerateVariationForArchetype(ECharacterArchetype Archetype, ECharacterAge Age)
{
    FCharacterVariation Variation;
    
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            // Paleontologist - clean, academic appearance initially
            Variation.SkinTone = FMath::RandRange(0.3f, 0.7f);
            Variation.BodyBuild = FMath::RandRange(0.3f, 0.5f); // Not heavily muscled
            Variation.WearLevel = 0.1f; // Starts clean
            Variation.ScarLevel = 0.0f; // No scars initially
            Variation.DirtLevel = 0.1f; // Clean
            break;
            
        case ECharacterArchetype::TribalHunter:
            // Hunters are muscular, scarred, weathered
            Variation.BodyBuild = FMath::RandRange(0.6f, 0.9f);
            Variation.ScarLevel = FMath::RandRange(0.3f, 0.7f);
            Variation.WearLevel = FMath::RandRange(0.4f, 0.8f);
            Variation.DirtLevel = FMath::RandRange(0.5f, 0.8f);
            break;
            
        case ECharacterArchetype::TribalGatherer:
            // Gatherers are lean, practical, moderately worn
            Variation.BodyBuild = FMath::RandRange(0.3f, 0.6f);
            Variation.ScarLevel = FMath::RandRange(0.1f, 0.4f);
            Variation.WearLevel = FMath::RandRange(0.3f, 0.6f);
            Variation.DirtLevel = FMath::RandRange(0.4f, 0.7f);
            break;
            
        case ECharacterArchetype::TribalElder:
            // Elders show age, wisdom, many scars from long life
            Variation.BodyBuild = FMath::RandRange(0.2f, 0.5f); // Less muscular with age
            Variation.ScarLevel = FMath::RandRange(0.5f, 0.9f); // Many life experiences
            Variation.WearLevel = FMath::RandRange(0.6f, 0.9f); // Well-worn items
            break;
            
        case ECharacterArchetype::TribalShaman:
            // Shamans are mystical, decorated, unique markings
            Variation.ScarLevel = FMath::RandRange(0.2f, 0.6f); // Ritual markings/scars
            Variation.WearLevel = FMath::RandRange(0.3f, 0.7f); // Ceremonial wear
            break;
            
        case ECharacterArchetype::TribalChild:
            // Children are smaller, cleaner, fewer scars
            Variation.Height = FMath::RandRange(0.1f, 0.4f); // Shorter
            Variation.BodyBuild = FMath::RandRange(0.2f, 0.4f); // Not muscular
            Variation.ScarLevel = FMath::RandRange(0.0f, 0.2f); // Few scars
            Variation.WearLevel = FMath::RandRange(0.2f, 0.5f);
            break;
            
        default:
            // Default random variation
            break;
    }
    
    // Age-based modifications
    switch (Age)
    {
        case ECharacterAge::Child:
            Variation.Height *= 0.6f; // Children are shorter
            Variation.BodyBuild *= 0.5f; // Less muscle mass
            Variation.ScarLevel *= 0.3f; // Fewer scars
            break;
        case ECharacterAge::Elder:
            Variation.BodyBuild *= 0.8f; // Muscle loss with age
            Variation.ScarLevel = FMath::Min(Variation.ScarLevel + 0.3f, 1.0f); // More life experience
            break;
        default:
            break;
    }
    
    return Variation;
}

FString UCharacterArtistSystem::GenerateCharacterName(ECharacterGender Gender, ECharacterArchetype Archetype)
{
    TArray<FString> MaleNames = {
        "Kael", "Thane", "Brok", "Daven", "Gareth", "Jorik", "Mael", "Nolan", "Orin", "Raven",
        "Soren", "Talon", "Ulric", "Vance", "Wren", "Zane", "Ash", "Blade", "Cade", "Drake"
    };
    
    TArray<FString> FemaleNames = {
        "Aria", "Brynn", "Cora", "Dara", "Elara", "Faye", "Gwen", "Hana", "Isla", "Jora",
        "Kira", "Luna", "Mira", "Naia", "Ora", "Pira", "Quinn", "Rhea", "Sage", "Tara"
    };
    
    // Special case for protagonist
    if (Archetype == ECharacterArchetype::Protagonist)
    {
        return "Dr. Marcus Thorne"; // Temporary - will be set by Miguel
    }
    
    FString BaseName;
    if (Gender == ECharacterGender::Male)
    {
        BaseName = MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
    }
    else
    {
        BaseName = FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
    }
    
    return BaseName;
}

TArray<FString> UCharacterArtistSystem::GeneratePersonalityTraits(ECharacterArchetype Archetype)
{
    TArray<FString> Traits;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
            Traits = {"Brave", "Protective", "Skilled tracker", "Quick reflexes", "Pack loyalty"};
            break;
        case ECharacterArchetype::TribalGatherer:
            Traits = {"Observant", "Patient", "Resourceful", "Nurturing", "Knowledge of plants"};
            break;
        case ECharacterArchetype::TribalElder:
            Traits = {"Wise", "Experienced", "Storyteller", "Cautious", "Respected leader"};
            break;
        case ECharacterArchetype::TribalShaman:
            Traits = {"Mystical", "Intuitive", "Healer", "Spiritual guide", "Keeper of traditions"};
            break;
        case ECharacterArchetype::TribalChild:
            Traits = {"Curious", "Energetic", "Quick learner", "Innocent", "Adaptable"};
            break;
        case ECharacterArchetype::Survivor:
            Traits = {"Resilient", "Self-reliant", "Paranoid", "Hardened", "Survivor instincts"};
            break;
        case ECharacterArchetype::Wanderer:
            Traits = {"Independent", "Knowledgeable", "Mysterious", "Well-traveled", "Adaptable"};
            break;
        default:
            Traits = {"Determined", "Adaptable", "Resourceful"};
            break;
    }
    
    return Traits;
}

FText UCharacterArtistSystem::GenerateBackgroundStory(ECharacterArchetype Archetype, ECharacterGender Gender)
{
    FString Story;
    FString Pronoun = (Gender == ECharacterGender::Male) ? "He" : "She";
    FString PronounLower = (Gender == ECharacterGender::Male) ? "he" : "she";
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
            Story = FString::Printf(TEXT("A skilled hunter who has provided for the tribe since youth. %s knows the hunting grounds better than anyone and has faced down predators that would terrify most. The scars on %s body tell stories of close encounters with the great beasts."), *Pronoun, *PronounLower);
            break;
        case ECharacterArchetype::TribalGatherer:
            Story = FString::Printf(TEXT("An expert in finding sustenance in the harsh wilderness. %s can identify edible plants, locate water sources, and knows which materials are best for crafting. %s knowledge has kept the tribe fed through many difficult seasons."), *Pronoun, *Pronoun);
            break;
        case ECharacterArchetype::TribalElder:
            Story = FString::Printf(TEXT("A repository of tribal wisdom and experience. %s has lived through many seasons and remembers when the great migrations began. %s guidance has helped the tribe survive countless dangers and %s stories preserve their history."), *Pronoun, *Pronoun, *PronounLower);
            break;
        case ECharacterArchetype::TribalShaman:
            Story = FString::Printf(TEXT("The spiritual heart of the tribe, keeper of ancient rituals and healing knowledge. %s communes with the spirits of the land and tends to both physical and spiritual wounds. The tribe looks to %s for guidance in times of uncertainty."), *Pronoun, *PronounLower);
            break;
        case ECharacterArchetype::TribalChild:
            Story = FString::Printf(TEXT("Born into this dangerous world, %s has known no other life than one of constant vigilance and survival. Despite the harsh realities, %s maintains the curiosity and wonder that comes with youth, learning from the adults around %s."), *PronounLower, *PronounLower, *PronounLower);
            break;
        case ECharacterArchetype::Survivor:
            Story = FString::Printf(TEXT("A lone survivor whose tribe was lost to the great predators. %s has learned to trust only %s own skills and instincts. Every day is a battle for survival, and %s has become hardened by the constant struggle."), *Pronoun, *PronounLower, *PronounLower);
            break;
        case ECharacterArchetype::Wanderer:
            Story = FString::Printf(TEXT("A traveler who moves between the scattered communities, carrying news and trading goods. %s has seen more of this dangerous world than most and knows the safest paths through predator territories."), *Pronoun);
            break;
        default:
            Story = TEXT("A person shaped by the harsh realities of survival in the age of great beasts.");
            break;
    }
    
    return FText::FromString(Story);
}

bool UCharacterArtistSystem::ValidateCharacterDiversity(const FCharacterDefinition& NewCharacter)
{
    if (!bEnsureVisualDiversity)
    {
        return true;
    }
    
    for (const FCharacterDefinition& ExistingCharacter : CharacterDatabase)
    {
        // Calculate visual similarity
        float SimilarityScore = 0.0f;
        const FCharacterVariation& NewVar = NewCharacter.PhysicalVariation;
        const FCharacterVariation& ExistingVar = ExistingCharacter.PhysicalVariation;
        
        SimilarityScore += FMath::Abs(NewVar.SkinTone - ExistingVar.SkinTone);
        SimilarityScore += FMath::Abs(NewVar.BodyBuild - ExistingVar.BodyBuild);
        SimilarityScore += FMath::Abs(NewVar.Height - ExistingVar.Height);
        SimilarityScore /= 3.0f; // Average
        
        if (SimilarityScore < MinimumVariationThreshold)
        {
            return false; // Too similar to existing character
        }
    }
    
    return true;
}

void UCharacterArtistSystem::SaveCharacterToDatabase(const FCharacterDefinition& Character)
{
    if (ValidateCharacterDiversity(Character))
    {
        CharacterDatabase.Add(Character);
        
        if (CharacterDatabase.Num() > MaxUniqueCharacters)
        {
            // Remove oldest character to maintain limit
            CharacterDatabase.RemoveAt(0);
        }
    }
}

FCharacterDefinition UCharacterArtistSystem::GetCharacterByName(const FString& Name)
{
    for (const FCharacterDefinition& Character : CharacterDatabase)
    {
        if (Character.CharacterName == Name)
        {
            return Character;
        }
    }
    
    // Return empty character if not found
    return FCharacterDefinition();
}