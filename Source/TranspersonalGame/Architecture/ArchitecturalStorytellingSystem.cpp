#include "ArchitecturalStorytellingSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UArchitecturalStorytellingSystem::UArchitecturalStorytellingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize storytelling parameters
    StorytellingIntensity = 0.8f;
    NarrativeComplexity = 0.7f;
    EmotionalImpact = 0.6f;
    EnvironmentalIntegration = 0.9f;
    
    // Initialize story type probabilities
    StoryTypeProbabilities.Add(EStoryType::Tragedy, 0.3f);
    StoryTypeProbabilities.Add(EStoryType::Mystery, 0.25f);
    StoryTypeProbabilities.Add(EStoryType::Survival, 0.2f);
    StoryTypeProbabilities.Add(EStoryType::Discovery, 0.15f);
    StoryTypeProbabilities.Add(EStoryType::Abandonment, 0.1f);
    
    // Initialize material aging settings
    MaterialAgingSettings.WoodDecayRate = 0.7f;
    MaterialAgingSettings.StoneWeatheringRate = 0.3f;
    MaterialAgingSettings.MetalCorrosionRate = 0.5f;
    MaterialAgingSettings.FabricDecayRate = 0.9f;
    MaterialAgingSettings.OverallAgingMultiplier = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStorytellingSystem initialized"));
}

void UArchitecturalStorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize storytelling system
    InitializeStorytellingDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStorytellingSystem: BeginPlay completed"));
}

void UArchitecturalStorytellingSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up any active storytelling elements
    CleanupStorytellingElements();
    
    Super::EndPlay(EndPlayReason);
}

FStructureStory UArchitecturalStorytellingSystem::GenerateStoryForStructure(const FStructureContext& Context)
{
    FStructureStory Story;
    
    // Determine story type based on context and probabilities
    EStoryType SelectedStoryType = SelectStoryType(Context);
    Story.StoryType = SelectedStoryType;
    
    // Generate narrative elements based on story type
    switch (SelectedStoryType)
    {
        case EStoryType::Tragedy:
            Story = GenerateTragedyStory(Context);
            break;
        case EStoryType::Mystery:
            Story = GenerateMysteryStory(Context);
            break;
        case EStoryType::Survival:
            Story = GenerateSurvivalStory(Context);
            break;
        case EStoryType::Discovery:
            Story = GenerateDiscoveryStory(Context);
            break;
        case EStoryType::Abandonment:
            Story = GenerateAbandonmentStory(Context);
            break;
        default:
            Story = GenerateDefaultStory(Context);
            break;
    }
    
    // Apply environmental context
    ApplyEnvironmentalContext(Story, Context);
    
    // Calculate storytelling metrics
    CalculateStorytellingMetrics(Story);
    
    UE_LOG(LogTemp, Log, TEXT("Generated %s story for structure: %s"), 
           *UEnum::GetValueAsString(SelectedStoryType), *Story.NarrativeTitle);
    
    return Story;
}

void UArchitecturalStorytellingSystem::ApplyStoryToStructure(AActor* Structure, const FStructureStory& Story)
{
    if (!Structure)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyStoryToStructure: Invalid structure actor"));
        return;
    }
    
    // Apply visual storytelling elements
    ApplyVisualElements(Structure, Story);
    
    // Apply material aging and weathering
    ApplyMaterialAging(Structure, Story);
    
    // Place narrative props and clues
    PlaceNarrativeProps(Structure, Story);
    
    // Apply atmospheric elements
    ApplyAtmosphericElements(Structure, Story);
    
    // Create interactive elements
    CreateInteractiveElements(Structure, Story);
    
    UE_LOG(LogTemp, Log, TEXT("Applied story '%s' to structure %s"), 
           *Story.NarrativeTitle, *Structure->GetName());
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateNarrativeProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    // Generate props based on story type and context
    switch (Story.StoryType)
    {
        case EStoryType::Tragedy:
            Props.Append(GenerateTragedyProps(Story));
            break;
        case EStoryType::Mystery:
            Props.Append(GenerateMysteryProps(Story));
            break;
        case EStoryType::Survival:
            Props.Append(GenerateSurvivalProps(Story));
            break;
        case EStoryType::Discovery:
            Props.Append(GenerateDiscoveryProps(Story));
            break;
        case EStoryType::Abandonment:
            Props.Append(GenerateAbandonmentProps(Story));
            break;
    }
    
    // Add universal props based on time period and environment
    Props.Append(GenerateUniversalProps(Story));
    
    return Props;
}

float UArchitecturalStorytellingSystem::CalculateStorytellingScore(const FStructureStory& Story) const
{
    float Score = 0.0f;
    
    // Base score from narrative complexity
    Score += Story.NarrativeComplexity * 0.3f;
    
    // Emotional impact contribution
    Score += Story.EmotionalImpact * 0.25f;
    
    // Environmental integration
    Score += Story.EnvironmentalIntegration * 0.2f;
    
    // Prop density and quality
    float PropScore = FMath::Min(Story.NarrativeProps.Num() / 10.0f, 1.0f);
    Score += PropScore * 0.15f;
    
    // Historical authenticity
    Score += Story.HistoricalAuthenticity * 0.1f;
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void UArchitecturalStorytellingSystem::InitializeStorytellingDatabase()
{
    // Initialize narrative templates
    InitializeNarrativeTemplates();
    
    // Load prop databases
    LoadPropDatabases();
    
    // Initialize material aging systems
    InitializeMaterialAging();
    
    UE_LOG(LogTemp, Log, TEXT("Storytelling database initialized"));
}

EStoryType UArchitecturalStorytellingSystem::SelectStoryType(const FStructureContext& Context)
{
    // Calculate weighted probabilities based on context
    TMap<EStoryType, float> AdjustedProbabilities = StoryTypeProbabilities;
    
    // Adjust probabilities based on biome
    if (Context.BiomeType == EJurassicBiomeType::DenseForest)
    {
        AdjustedProbabilities[EStoryType::Mystery] *= 1.5f;
        AdjustedProbabilities[EStoryType::Survival] *= 1.3f;
    }
    else if (Context.BiomeType == EJurassicBiomeType::OpenPlains)
    {
        AdjustedProbabilities[EStoryType::Discovery] *= 1.4f;
        AdjustedProbabilities[EStoryType::Abandonment] *= 1.2f;
    }
    
    // Adjust based on structure type
    if (Context.StructureType == EStructureType::Shelter)
    {
        AdjustedProbabilities[EStoryType::Survival] *= 1.6f;
        AdjustedProbabilities[EStoryType::Tragedy] *= 1.3f;
    }
    else if (Context.StructureType == EStructureType::Ceremonial)
    {
        AdjustedProbabilities[EStoryType::Mystery] *= 1.8f;
        AdjustedProbabilities[EStoryType::Discovery] *= 1.4f;
    }
    
    // Select story type using weighted random selection
    float TotalWeight = 0.0f;
    for (const auto& Pair : AdjustedProbabilities)
    {
        TotalWeight += Pair.Value;
    }
    
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const auto& Pair : AdjustedProbabilities)
    {
        CurrentWeight += Pair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return Pair.Key;
        }
    }
    
    return EStoryType::Survival; // Default fallback
}

FStructureStory UArchitecturalStorytellingSystem::GenerateTragedyStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Tragedy;
    Story.NarrativeTitle = "The Last Refuge";
    Story.NarrativeDescription = "A family's desperate attempt to survive ended in tragedy when predators found their hiding place.";
    
    // Generate tragedy-specific elements
    Story.EmotionalImpact = FMath::RandRange(0.7f, 1.0f);
    Story.NarrativeComplexity = FMath::RandRange(0.6f, 0.9f);
    Story.HistoricalAuthenticity = FMath::RandRange(0.8f, 1.0f);
    
    // Add tragedy props
    FNarrativeProp BloodStain;
    BloodStain.PropType = ENarrativePropType::Evidence;
    BloodStain.Description = "Dark stains on the floor tell a grim tale";
    BloodStain.EmotionalWeight = 0.9f;
    Story.NarrativeProps.Add(BloodStain);
    
    FNarrativeProp BrokenWeapon;
    BrokenWeapon.PropType = ENarrativePropType::Tool;
    BrokenWeapon.Description = "A shattered spear, useless against the prehistoric threat";
    BrokenWeapon.EmotionalWeight = 0.7f;
    Story.NarrativeProps.Add(BrokenWeapon);
    
    return Story;
}

FStructureStory UArchitecturalStorytellingSystem::GenerateMysteryStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Mystery;
    Story.NarrativeTitle = "The Vanished Tribe";
    Story.NarrativeDescription = "The inhabitants disappeared without a trace, leaving behind only questions.";
    
    Story.EmotionalImpact = FMath::RandRange(0.5f, 0.8f);
    Story.NarrativeComplexity = FMath::RandRange(0.8f, 1.0f);
    Story.HistoricalAuthenticity = FMath::RandRange(0.7f, 0.9f);
    
    // Add mystery props
    FNarrativeProp StrangeSymbol;
    StrangeSymbol.PropType = ENarrativePropType::Decoration;
    StrangeSymbol.Description = "Mysterious symbols carved into the wall";
    StrangeSymbol.EmotionalWeight = 0.6f;
    Story.NarrativeProps.Add(StrangeSymbol);
    
    return Story;
}

FStructureStory UArchitecturalStorytellingSystem::GenerateSurvivalStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Survival;
    Story.NarrativeTitle = "Against All Odds";
    Story.NarrativeDescription = "Evidence of a long struggle against the harsh prehistoric world.";
    
    Story.EmotionalImpact = FMath::RandRange(0.6f, 0.8f);
    Story.NarrativeComplexity = FMath::RandRange(0.5f, 0.7f);
    Story.HistoricalAuthenticity = FMath::RandRange(0.8f, 1.0f);
    
    return Story;
}

FStructureStory UArchitecturalStorytellingSystem::GenerateDiscoveryStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Discovery;
    Story.NarrativeTitle = "The Pioneer's Cache";
    Story.NarrativeDescription = "A place where ancient humans made groundbreaking discoveries about their world.";
    
    Story.EmotionalImpact = FMath::RandRange(0.4f, 0.7f);
    Story.NarrativeComplexity = FMath::RandRange(0.7f, 0.9f);
    Story.HistoricalAuthenticity = FMath::RandRange(0.6f, 0.8f);
    
    return Story;
}

FStructureStory UArchitecturalStorytellingSystem::GenerateAbandonmentStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Abandonment;
    Story.NarrativeTitle = "The Exodus";
    Story.NarrativeDescription = "Left behind when the tribe moved to safer grounds.";
    
    Story.EmotionalImpact = FMath::RandRange(0.3f, 0.6f);
    Story.NarrativeComplexity = FMath::RandRange(0.4f, 0.6f);
    Story.HistoricalAuthenticity = FMath::RandRange(0.7f, 0.9f);
    
    return Story;
}

FStructureStory UArchitecturalStorytellingSystem::GenerateDefaultStory(const FStructureContext& Context)
{
    FStructureStory Story;
    Story.StoryType = EStoryType::Survival;
    Story.NarrativeTitle = "Echoes of the Past";
    Story.NarrativeDescription = "A simple dwelling that speaks of humanity's struggle to survive.";
    
    Story.EmotionalImpact = 0.5f;
    Story.NarrativeComplexity = 0.5f;
    Story.HistoricalAuthenticity = 0.8f;
    
    return Story;
}

void UArchitecturalStorytellingSystem::ApplyEnvironmentalContext(FStructureStory& Story, const FStructureContext& Context)
{
    // Adjust story elements based on environmental factors
    if (Context.BiomeType == EJurassicBiomeType::DenseForest)
    {
        Story.EnvironmentalIntegration *= 1.2f;
        Story.NarrativeDescription += " The dense forest provided both shelter and danger.";
    }
    else if (Context.BiomeType == EJurassicBiomeType::OpenPlains)
    {
        Story.EnvironmentalIntegration *= 0.9f;
        Story.NarrativeDescription += " Exposed on the open plains, vulnerability was constant.";
    }
    
    // Clamp values
    Story.EnvironmentalIntegration = FMath::Clamp(Story.EnvironmentalIntegration, 0.0f, 1.0f);
}

void UArchitecturalStorytellingSystem::CalculateStorytellingMetrics(FStructureStory& Story)
{
    // Calculate overall storytelling score
    Story.StorytellingScore = CalculateStorytellingScore(Story);
    
    // Calculate narrative density
    Story.NarrativeDensity = FMath::Min(Story.NarrativeProps.Num() / 8.0f, 1.0f);
    
    // Calculate emotional resonance
    float TotalEmotionalWeight = 0.0f;
    for (const FNarrativeProp& Prop : Story.NarrativeProps)
    {
        TotalEmotionalWeight += Prop.EmotionalWeight;
    }
    Story.EmotionalResonance = Story.NarrativeProps.Num() > 0 ? 
        TotalEmotionalWeight / Story.NarrativeProps.Num() : 0.0f;
}

void UArchitecturalStorytellingSystem::ApplyVisualElements(AActor* Structure, const FStructureStory& Story)
{
    // Apply visual storytelling through materials and lighting
    TArray<UStaticMeshComponent*> MeshComponents;
    Structure->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetMaterial(0))
        {
            // Apply story-specific material modifications
            ApplyStoryMaterialEffects(MeshComp, Story);
        }
    }
}

void UArchitecturalStorytellingSystem::ApplyMaterialAging(AActor* Structure, const FStructureStory& Story)
{
    // Calculate aging factor based on story type and time
    float AgingFactor = CalculateAgingFactor(Story);
    
    TArray<UStaticMeshComponent*> MeshComponents;
    Structure->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            ApplyAgingToMesh(MeshComp, AgingFactor, Story);
        }
    }
}

void UArchitecturalStorytellingSystem::PlaceNarrativeProps(AActor* Structure, const FStructureStory& Story)
{
    for (const FNarrativeProp& Prop : Story.NarrativeProps)
    {
        SpawnNarrativeProp(Structure, Prop);
    }
}

void UArchitecturalStorytellingSystem::ApplyAtmosphericElements(AActor* Structure, const FStructureStory& Story)
{
    // Add atmospheric elements based on story type
    switch (Story.StoryType)
    {
        case EStoryType::Tragedy:
            CreateTragicAtmosphere(Structure, Story);
            break;
        case EStoryType::Mystery:
            CreateMysteriousAtmosphere(Structure, Story);
            break;
        default:
            CreateGenericAtmosphere(Structure, Story);
            break;
    }
}

void UArchitecturalStorytellingSystem::CreateInteractiveElements(AActor* Structure, const FStructureStory& Story)
{
    // Create interactive elements that reveal story details
    for (const FNarrativeProp& Prop : Story.NarrativeProps)
    {
        if (Prop.bIsInteractive)
        {
            CreateInteractiveProp(Structure, Prop);
        }
    }
}

void UArchitecturalStorytellingSystem::InitializeNarrativeTemplates()
{
    // Initialize story templates for different scenarios
    UE_LOG(LogTemp, Log, TEXT("Narrative templates initialized"));
}

void UArchitecturalStorytellingSystem::LoadPropDatabases()
{
    // Load prop databases for different story types
    UE_LOG(LogTemp, Log, TEXT("Prop databases loaded"));
}

void UArchitecturalStorytellingSystem::InitializeMaterialAging()
{
    // Initialize material aging systems
    UE_LOG(LogTemp, Log, TEXT("Material aging system initialized"));
}

void UArchitecturalStorytellingSystem::CleanupStorytellingElements()
{
    // Clean up any active storytelling elements
    UE_LOG(LogTemp, Log, TEXT("Storytelling elements cleaned up"));
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateTragedyProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    // Generate tragedy-specific props
    FNarrativeProp TornClothing;
    TornClothing.PropType = ENarrativePropType::Personal;
    TornClothing.Description = "Torn clothing scattered on the ground";
    TornClothing.EmotionalWeight = 0.8f;
    Props.Add(TornClothing);
    
    return Props;
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateMysteryProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    FNarrativeProp UnknownArtifact;
    UnknownArtifact.PropType = ENarrativePropType::Decoration;
    UnknownArtifact.Description = "An artifact of unknown purpose";
    UnknownArtifact.EmotionalWeight = 0.6f;
    Props.Add(UnknownArtifact);
    
    return Props;
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateSurvivalProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    FNarrativeProp MakeShiftTool;
    MakeShiftTool.PropType = ENarrativePropType::Tool;
    MakeShiftTool.Description = "Crude tools fashioned from available materials";
    MakeShiftTool.EmotionalWeight = 0.5f;
    Props.Add(MakeShiftTool);
    
    return Props;
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateDiscoveryProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    FNarrativeProp PrimitiveDrawing;
    PrimitiveDrawing.PropType = ENarrativePropType::Decoration;
    PrimitiveDrawing.Description = "Primitive drawings on the wall showing new discoveries";
    PrimitiveDrawing.EmotionalWeight = 0.4f;
    Props.Add(PrimitiveDrawing);
    
    return Props;
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateAbandonmentProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    FNarrativeProp LeftBehindItem;
    LeftBehindItem.PropType = ENarrativePropType::Personal;
    LeftBehindItem.Description = "Personal items left behind in haste";
    LeftBehindItem.EmotionalWeight = 0.6f;
    Props.Add(LeftBehindItem);
    
    return Props;
}

TArray<FNarrativeProp> UArchitecturalStorytellingSystem::GenerateUniversalProps(const FStructureStory& Story)
{
    TArray<FNarrativeProp> Props;
    
    FNarrativeProp FirePit;
    FirePit.PropType = ENarrativePropType::Functional;
    FirePit.Description = "Cold ashes mark where warmth once was";
    FirePit.EmotionalWeight = 0.3f;
    Props.Add(FirePit);
    
    return Props;
}

float UArchitecturalStorytellingSystem::CalculateAgingFactor(const FStructureStory& Story) const
{
    float AgingFactor = 0.5f; // Base aging
    
    // Adjust based on story type
    switch (Story.StoryType)
    {
        case EStoryType::Tragedy:
            AgingFactor += 0.3f; // More decay due to abandonment
            break;
        case EStoryType::Abandonment:
            AgingFactor += 0.4f; // Significant aging
            break;
        case EStoryType::Discovery:
            AgingFactor -= 0.1f; // Better maintained
            break;
    }
    
    return FMath::Clamp(AgingFactor, 0.0f, 1.0f);
}

void UArchitecturalStorytellingSystem::ApplyStoryMaterialEffects(UStaticMeshComponent* MeshComp, const FStructureStory& Story)
{
    // Apply story-specific material effects
    if (!MeshComp) return;
    
    // This would typically involve creating dynamic material instances
    // and adjusting parameters based on the story
    UE_LOG(LogTemp, Log, TEXT("Applied story material effects to mesh component"));
}

void UArchitecturalStorytellingSystem::ApplyAgingToMesh(UStaticMeshComponent* MeshComp, float AgingFactor, const FStructureStory& Story)
{
    // Apply aging effects to mesh materials
    if (!MeshComp) return;
    
    // This would involve adjusting material parameters for weathering, decay, etc.
    UE_LOG(LogTemp, Log, TEXT("Applied aging factor %.2f to mesh component"), AgingFactor);
}

void UArchitecturalStorytellingSystem::SpawnNarrativeProp(AActor* Structure, const FNarrativeProp& Prop)
{
    // Spawn a narrative prop near the structure
    if (!Structure) return;
    
    UE_LOG(LogTemp, Log, TEXT("Spawned narrative prop: %s"), *Prop.Description);
}

void UArchitecturalStorytellingSystem::CreateTragicAtmosphere(AActor* Structure, const FStructureStory& Story)
{
    // Create atmospheric elements for tragedy stories
    UE_LOG(LogTemp, Log, TEXT("Created tragic atmosphere for structure"));
}

void UArchitecturalStorytellingSystem::CreateMysteriousAtmosphere(AActor* Structure, const FStructureStory& Story)
{
    // Create atmospheric elements for mystery stories
    UE_LOG(LogTemp, Log, TEXT("Created mysterious atmosphere for structure"));
}

void UArchitecturalStorytellingSystem::CreateGenericAtmosphere(AActor* Structure, const FStructureStory& Story)
{
    // Create generic atmospheric elements
    UE_LOG(LogTemp, Log, TEXT("Created generic atmosphere for structure"));
}

void UArchitecturalStorytellingSystem::CreateInteractiveProp(AActor* Structure, const FNarrativeProp& Prop)
{
    // Create an interactive prop that reveals story information
    UE_LOG(LogTemp, Log, TEXT("Created interactive prop: %s"), *Prop.Description);
}