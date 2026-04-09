// Copyright Transpersonal Game Studio. All Rights Reserved.
// Jurassic Architecture Manager Implementation
// Agent #07 - Architecture & Interior Agent

#include "JurassicArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogJurassicArchitecture, Log, All);

AJurassicArchitectureManager::AJurassicArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    
    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create PCG component for procedural generation
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    PCGComponent->SetupAttachment(RootComponent);
    
    // Initialize default values
    MaxStructuresPerBiome = 10;
    MinDistanceBetweenStructures = 2000.0f; // 20 meters
    MaxDistanceBetweenStructures = 10000.0f; // 100 meters
    StructureSpawnRadius = 50000.0f; // 500 meters
    bAutoGenerateStructures = true;
    bUseStorytellingSystem = true;
    GenerationSeed = 12345;
    
    // Performance settings
    MaxActiveStructures = 50;
    CullDistance = 100000.0f; // 1km
    DetailCullDistance = 50000.0f; // 500m
    
    // Initialize arrays
    ActiveStructures.Empty();
    StructureTemplates.Empty();
    BiomeManagers.Empty();
}

void AJurassicArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Jurassic Architecture Manager starting up..."));
    
    // Initialize the random stream with seed
    RandomStream.Initialize(GenerationSeed);
    
    // Find biome managers in the world
    InitializeBiomeManagers();
    
    // Load structure templates
    LoadStructureTemplates();
    
    // Start generation if auto-generation is enabled
    if (bAutoGenerateStructures)
    {
        StartStructureGeneration();
    }
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Architecture Manager initialized with %d templates"), StructureTemplates.Num());
}

void AJurassicArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update structure visibility based on player distance
    UpdateStructureVisibility();
    
    // Check for new generation opportunities
    if (bAutoGenerateStructures)
    {
        CheckForGenerationOpportunities();
    }
    
    // Update storytelling elements
    if (bUseStorytellingSystem)
    {
        UpdateStorytellingElements(DeltaTime);
    }
}

void AJurassicArchitectureManager::InitializeBiomeManagers()
{
    BiomeManagers.Empty();
    
    // Find all biome managers in the world
    TArray<AActor*> FoundBiomeManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AJurassicBiomeManager::StaticClass(), FoundBiomeManagers);
    
    for (AActor* Actor : FoundBiomeManagers)
    {
        if (AJurassicBiomeManager* BiomeManager = Cast<AJurassicBiomeManager>(Actor))
        {
            BiomeManagers.Add(BiomeManager);
            UE_LOG(LogJurassicArchitecture, Log, TEXT("Found biome manager: %s"), *BiomeManager->GetName());
        }
    }
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Initialized with %d biome managers"), BiomeManagers.Num());
}

void AJurassicArchitectureManager::LoadStructureTemplates()
{
    // Create default structure templates if none are configured
    if (StructureTemplates.Num() == 0)
    {
        CreateDefaultStructureTemplates();
    }
    
    // Validate all templates
    for (int32 i = StructureTemplates.Num() - 1; i >= 0; i--)
    {
        if (!ValidateStructureTemplate(StructureTemplates[i]))
        {
            UE_LOG(LogJurassicArchitecture, Warning, TEXT("Removing invalid structure template at index %d"), i);
            StructureTemplates.RemoveAt(i);
        }
    }
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Loaded %d valid structure templates"), StructureTemplates.Num());
}

void AJurassicArchitectureManager::CreateDefaultStructureTemplates()
{
    // Basic Shelter Template
    FStructureTemplate BasicShelter;
    BasicShelter.TemplateName = "Basic Survival Shelter";
    BasicShelter.StructureType = EStructureType::BasicShelter;
    BasicShelter.ConstructionMethod = EConstructionTechnique::Lean_To;
    BasicShelter.BaseDimensions = FVector(400.0f, 300.0f, 250.0f);
    BasicShelter.SizeVariation = FVector2D(0.8f, 1.2f);
    BasicShelter.BaseIntegrity = EStructuralIntegrity::Good;
    BasicShelter.SpawnProbability = 0.4f;
    BasicShelter.MaxInstancesPerBiome = 5;
    BasicShelter.bRequiresFlatGround = true;
    BasicShelter.bAvoidDenseVegetation = true;
    BasicShelter.MinWaterDistance = 100.0f;
    BasicShelter.MaxWaterDistance = 1000.0f;
    
    // Create basic story for this shelter
    FArchitecturalStory BasicStory;
    BasicStory.InhabitantType = EInhabitationStory::ActiveFamily;
    BasicStory.InhabitantCount = 2;
    BasicStory.InhabitationDays = 15;
    BasicStory.DepartureReason = "Moved to find better hunting grounds";
    BasicStory.StoryClues.Add("Worn sleeping furs");
    BasicStory.StoryClues.Add("Simple cooking stones");
    BasicStory.StoryClues.Add("Tool sharpening marks");
    BasicStory.EmotionalTone = "Hopeful but cautious";
    BasicStory.bHasDefensiveModifications = false;
    
    BasicShelter.PossibleStories.Add(BasicStory);
    StructureTemplates.Add(BasicShelter);
    
    // Family Cave Template
    FStructureTemplate FamilyCave;
    FamilyCave.TemplateName = "Extended Family Cave";
    FamilyCave.StructureType = EStructureType::FamilyCave;
    FamilyCave.ConstructionMethod = EConstructionTechnique::Cave_Modification;
    FamilyCave.BaseDimensions = FVector(800.0f, 600.0f, 400.0f);
    FamilyCave.SizeVariation = FVector2D(0.9f, 1.3f);
    FamilyCave.BaseIntegrity = EStructuralIntegrity::Excellent;
    FamilyCave.SpawnProbability = 0.2f;
    FamilyCave.MaxInstancesPerBiome = 2;
    FamilyCave.bRequiresFlatGround = false;
    FamilyCave.MinWaterDistance = 50.0f;
    FamilyCave.MaxWaterDistance = 500.0f;
    
    // Create family story
    FArchitecturalStory FamilyStory;
    FamilyStory.InhabitantType = EInhabitationStory::ActiveFamily;
    FamilyStory.InhabitantCount = 6;
    FamilyStory.InhabitationDays = 120;
    FamilyStory.DepartureReason = "Seasonal migration";
    FamilyStory.StoryClues.Add("Multiple sleeping areas");
    FamilyStory.StoryClues.Add("Children's toys and games");
    FamilyStory.StoryClues.Add("Food storage containers");
    FamilyStory.StoryClues.Add("Family gathering circle");
    FamilyStory.EmotionalTone = "Warm and protective";
    FamilyStory.bHasDefensiveModifications = true;
    
    FamilyCave.PossibleStories.Add(FamilyStory);
    StructureTemplates.Add(FamilyCave);
    
    // Abandoned Ruins Template
    FStructureTemplate AbandonedRuins;
    AbandonedRuins.TemplateName = "Mysterious Abandoned Ruins";
    AbandonedRuins.StructureType = EStructureType::AbandonedRuins;
    AbandonedRuins.ConstructionMethod = EConstructionTechnique::Stone_Circle;
    AbandonedRuins.BaseDimensions = FVector(1000.0f, 1000.0f, 300.0f);
    AbandonedRuins.SizeVariation = FVector2D(0.7f, 1.0f);
    AbandonedRuins.BaseIntegrity = EStructuralIntegrity::Poor;
    AbandonedRuins.SpawnProbability = 0.1f;
    AbandonedRuins.MaxInstancesPerBiome = 1;
    AbandonedRuins.bRequiresFlatGround = false;
    AbandonedRuins.MinWaterDistance = 200.0f;
    AbandonedRuins.MaxWaterDistance = 2000.0f;
    
    // Create mystery story
    FArchitecturalStory MysteryStory;
    MysteryStory.InhabitantType = EInhabitationStory::MysteriousAbandonment;
    MysteryStory.InhabitantCount = 0;
    MysteryStory.InhabitationDays = 0;
    MysteryStory.DepartureReason = "Unknown - no signs of struggle or planned departure";
    MysteryStory.StoryClues.Add("Unfinished meal on table");
    MysteryStory.StoryClues.Add("Tools left mid-use");
    MysteryStory.StoryClues.Add("Strange energy residue");
    MysteryStory.StoryClues.Add("Unusual crystal formations");
    MysteryStory.EmotionalTone = "Eerie and mysterious";
    MysteryStory.bHasQuestItem = true;
    MysteryStory.bHasSecretArea = true;
    
    AbandonedRuins.PossibleStories.Add(MysteryStory);
    StructureTemplates.Add(AbandonedRuins);
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Created %d default structure templates"), StructureTemplates.Num());
}

bool AJurassicArchitectureManager::ValidateStructureTemplate(const FStructureTemplate& Template)
{
    // Check basic requirements
    if (Template.TemplateName.IsEmpty())
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Structure template missing name"));
        return false;
    }
    
    if (Template.BaseDimensions.IsZero())
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Structure template %s has zero dimensions"), *Template.TemplateName);
        return false;
    }
    
    if (Template.SpawnProbability <= 0.0f)
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Structure template %s has invalid spawn probability"), *Template.TemplateName);
        return false;
    }
    
    if (Template.PossibleStories.Num() == 0)
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Structure template %s has no stories defined"), *Template.TemplateName);
        return false;
    }
    
    return true;
}

void AJurassicArchitectureManager::StartStructureGeneration()
{
    if (BiomeManagers.Num() == 0)
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Cannot start generation - no biome managers found"));
        return;
    }
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Starting structure generation across %d biomes"), BiomeManagers.Num());
    
    for (AJurassicBiomeManager* BiomeManager : BiomeManagers)
    {
        if (IsValid(BiomeManager))
        {
            GenerateStructuresForBiome(BiomeManager);
        }
    }
}

void AJurassicArchitectureManager::GenerateStructuresForBiome(AJurassicBiomeManager* BiomeManager)
{
    if (!IsValid(BiomeManager))
    {
        return;
    }
    
    // Get biome bounds and type
    FVector BiomeCenter = BiomeManager->GetActorLocation();
    float BiomeRadius = 25000.0f; // 250 meters default
    EJurassicBiomeType BiomeType = EJurassicBiomeType::TemperateForest; // Default
    
    // Try to get actual biome data
    // Note: This would need to be implemented based on the actual BiomeManager interface
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Generating structures for biome at %s"), *BiomeCenter.ToString());
    
    // Generate structures based on templates
    for (const FStructureTemplate& Template : StructureTemplates)
    {
        // Check if this template is suitable for this biome
        if (Template.PreferredBiomes.Num() > 0 && !Template.PreferredBiomes.Contains(BiomeType))
        {
            continue;
        }
        
        // Calculate how many instances to spawn
        int32 NumToSpawn = FMath::RandRange(1, Template.MaxInstancesPerBiome);
        
        for (int32 i = 0; i < NumToSpawn; i++)
        {
            // Check spawn probability
            if (RandomStream.FRand() > Template.SpawnProbability)
            {
                continue;
            }
            
            // Find a suitable location
            FVector SpawnLocation;
            if (FindSuitableSpawnLocation(BiomeCenter, BiomeRadius, Template, SpawnLocation))
            {
                // Spawn the structure
                SpawnStructure(Template, SpawnLocation, BiomeManager);
            }
        }
    }
}

bool AJurassicArchitectureManager::FindSuitableSpawnLocation(const FVector& BiomeCenter, float BiomeRadius, 
    const FStructureTemplate& Template, FVector& OutLocation)
{
    const int32 MaxAttempts = 20;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        // Generate random location within biome
        FVector2D RandomCircle = RandomStream.GetUnitVector() * RandomStream.FRand() * BiomeRadius;
        FVector TestLocation = BiomeCenter + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
        
        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = TestLocation + FVector(0, 0, 1000.0f);
        FVector TraceEnd = TestLocation - FVector(0, 0, 1000.0f);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            TestLocation = HitResult.Location;
            
            // Check slope requirements
            FVector SurfaceNormal = HitResult.Normal;
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
            
            if (SlopeAngle < Template.MinSlopeAngle || SlopeAngle > Template.MaxSlopeAngle)
            {
                continue;
            }
            
            // Check distance from other structures
            bool bTooClose = false;
            for (const FActiveStructure& ExistingStructure : ActiveStructures)
            {
                float Distance = FVector::Dist(TestLocation, ExistingStructure.Location);
                if (Distance < MinDistanceBetweenStructures)
                {
                    bTooClose = true;
                    break;
                }
            }
            
            if (bTooClose)
            {
                continue;
            }
            
            // Additional checks could be added here (water distance, vegetation density, etc.)
            
            OutLocation = TestLocation;
            return true;
        }
    }
    
    return false;
}

APrehistoricStructure* AJurassicArchitectureManager::SpawnStructure(const FStructureTemplate& Template, 
    const FVector& Location, AJurassicBiomeManager* BiomeManager)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Spawn the structure actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    APrehistoricStructure* NewStructure = GetWorld()->SpawnActor<APrehistoricStructure>(
        APrehistoricStructure::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (NewStructure)
    {
        // Configure the structure based on template
        ConfigureStructureFromTemplate(NewStructure, Template);
        
        // Add to active structures list
        FActiveStructure ActiveStructure;
        ActiveStructure.Structure = NewStructure;
        ActiveStructure.Location = Location;
        ActiveStructure.Template = Template;
        ActiveStructure.BiomeManager = BiomeManager;
        ActiveStructure.SpawnTime = GetWorld()->GetTimeSeconds();
        ActiveStructure.bIsVisible = true;
        
        // Select a random story from the template
        if (Template.PossibleStories.Num() > 0)
        {
            int32 StoryIndex = RandomStream.RandRange(0, Template.PossibleStories.Num() - 1);
            ActiveStructure.Story = Template.PossibleStories[StoryIndex];
        }
        
        ActiveStructures.Add(ActiveStructure);
        
        UE_LOG(LogJurassicArchitecture, Log, TEXT("Spawned structure '%s' at %s"), 
            *Template.TemplateName, *Location.ToString());
        
        // Trigger storytelling setup
        if (bUseStorytellingSystem)
        {
            SetupStructureStorytelling(NewStructure, ActiveStructure.Story);
        }
    }
    
    return NewStructure;
}

void AJurassicArchitectureManager::ConfigureStructureFromTemplate(APrehistoricStructure* Structure, 
    const FStructureTemplate& Template)
{
    if (!IsValid(Structure))
    {
        return;
    }
    
    // Apply size variation
    FVector ScaledSize = Template.BaseDimensions;
    float ScaleFactor = RandomStream.FRandRange(Template.SizeVariation.X, Template.SizeVariation.Y);
    ScaledSize *= ScaleFactor;
    
    // Set structure properties
    // Note: These methods would need to be implemented in APrehistoricStructure
    // Structure->SetStructureType(Template.StructureType);
    // Structure->SetDimensions(ScaledSize);
    // Structure->SetConstructionMethod(Template.ConstructionMethod);
    // Structure->SetIntegrity(Template.BaseIntegrity);
    
    // Apply random rotation if allowed
    if (Template.bAllowRotation)
    {
        FRotator RandomRotation = FRotator(0, RandomStream.FRand() * 360.0f, 0);
        Structure->SetActorRotation(RandomRotation);
    }
    
    UE_LOG(LogJurassicArchitecture, Verbose, TEXT("Configured structure with scale factor %f"), ScaleFactor);
}

void AJurassicArchitectureManager::SetupStructureStorytelling(APrehistoricStructure* Structure, 
    const FArchitecturalStory& Story)
{
    if (!IsValid(Structure))
    {
        return;
    }
    
    // This would integrate with the storytelling system to place appropriate props,
    // wear patterns, and narrative elements based on the story
    
    UE_LOG(LogJurassicArchitecture, Verbose, TEXT("Setting up storytelling for structure: %s"), 
        *Story.EmotionalTone);
    
    // Example storytelling setup:
    // - Place props based on inhabitant type and count
    // - Add wear patterns based on inhabitation duration
    // - Create clues based on departure reason
    // - Set atmospheric elements based on emotional tone
    
    // This would be implemented in collaboration with the Narrative Agent
}

void AJurassicArchitectureManager::UpdateStructureVisibility()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update visibility for each active structure
    for (FActiveStructure& ActiveStructure : ActiveStructures)
    {
        if (!IsValid(ActiveStructure.Structure))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, ActiveStructure.Location);
        bool bShouldBeVisible = Distance <= CullDistance;
        
        if (bShouldBeVisible != ActiveStructure.bIsVisible)
        {
            ActiveStructure.Structure->SetActorHiddenInGame(!bShouldBeVisible);
            ActiveStructure.bIsVisible = bShouldBeVisible;
            
            if (bShouldBeVisible)
            {
                UE_LOG(LogJurassicArchitecture, Verbose, TEXT("Structure became visible at distance %f"), Distance);
            }
        }
        
        // Update detail level based on distance
        if (bShouldBeVisible)
        {
            bool bShowDetails = Distance <= DetailCullDistance;
            // This would control detail meshes, interior props, etc.
        }
    }
}

void AJurassicArchitectureManager::CheckForGenerationOpportunities()
{
    if (ActiveStructures.Num() >= MaxActiveStructures)
    {
        return;
    }
    
    // This could check for new areas the player has explored
    // and generate structures dynamically
    
    // For now, this is a placeholder for future dynamic generation
}

void AJurassicArchitectureManager::UpdateStorytellingElements(float DeltaTime)
{
    // Update time-based storytelling elements
    // - Weathering effects
    // - Ambient sounds
    // - Particle effects
    // - Dynamic lighting
    
    for (FActiveStructure& ActiveStructure : ActiveStructures)
    {
        if (IsValid(ActiveStructure.Structure) && ActiveStructure.bIsVisible)
        {
            // Update storytelling elements for visible structures
            UpdateStructureAtmosphere(ActiveStructure, DeltaTime);
        }
    }
}

void AJurassicArchitectureManager::UpdateStructureAtmosphere(FActiveStructure& Structure, float DeltaTime)
{
    // Update atmospheric elements like ambient sounds, particle effects, etc.
    // This would be implemented based on the story and emotional tone
    
    // Example: Play wind sounds for abandoned structures
    // Example: Add firefly particles for peaceful family homes
    // Example: Create eerie lighting for mysterious ruins
}

void AJurassicArchitectureManager::CleanupDistantStructures()
{
    if (!GetWorld())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float CleanupDistance = CullDistance * 2.0f; // Cleanup at double the cull distance
    
    for (int32 i = ActiveStructures.Num() - 1; i >= 0; i--)
    {
        FActiveStructure& ActiveStructure = ActiveStructures[i];
        
        if (!IsValid(ActiveStructure.Structure))
        {
            ActiveStructures.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, ActiveStructure.Location);
        if (Distance > CleanupDistance)
        {
            ActiveStructure.Structure->Destroy();
            ActiveStructures.RemoveAt(i);
            
            UE_LOG(LogJurassicArchitecture, Verbose, TEXT("Cleaned up distant structure at distance %f"), Distance);
        }
    }
}

// Blueprint callable functions
void AJurassicArchitectureManager::RegenerateAllStructures()
{
    // Clear existing structures
    for (FActiveStructure& ActiveStructure : ActiveStructures)
    {
        if (IsValid(ActiveStructure.Structure))
        {
            ActiveStructure.Structure->Destroy();
        }
    }
    ActiveStructures.Empty();
    
    // Regenerate with new seed
    GenerationSeed = FMath::Rand();
    RandomStream.Initialize(GenerationSeed);
    
    StartStructureGeneration();
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Regenerated all structures with new seed: %d"), GenerationSeed);
}

int32 AJurassicArchitectureManager::GetActiveStructureCount() const
{
    return ActiveStructures.Num();
}

TArray<APrehistoricStructure*> AJurassicArchitectureManager::GetNearbyStructures(const FVector& Location, float Radius) const
{
    TArray<APrehistoricStructure*> NearbyStructures;
    
    for (const FActiveStructure& ActiveStructure : ActiveStructures)
    {
        if (IsValid(ActiveStructure.Structure))
        {
            float Distance = FVector::Dist(Location, ActiveStructure.Location);
            if (Distance <= Radius)
            {
                NearbyStructures.Add(ActiveStructure.Structure);
            }
        }
    }
    
    return NearbyStructures;
}

void AJurassicArchitectureManager::SetGenerationSeed(int32 NewSeed)
{
    GenerationSeed = NewSeed;
    RandomStream.Initialize(GenerationSeed);
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Set new generation seed: %d"), GenerationSeed);
}