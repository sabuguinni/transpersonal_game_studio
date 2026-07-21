#include "DinosaurArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UDinosaurArchitecture::UDinosaurArchitecture()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    
    // Initialize dinosaur limits
    MaxTRexCount = 5;
    MaxVelociraptorCount = 20;
    MaxTriceratopsCount = 15;
    MaxBrachiosaurusCount = 8;
    MaxAnkylosaurusCount = 10;
    MaxParasaurolophusCount = 15;
    MaxOtherDinosaurCount = 10;
    MaxTotalDinosaurs = 150;
    
    // Initialize spawn settings
    bAutoSpawnDinosaurs = false;
    DinosaurSpawnRadius = 5000.0f;
    MinDistanceBetweenDinosaurs = 500.0f;
    
    // Initialize behavior settings
    bEnablePackBehavior = true;
    bEnableTerritorialBehavior = true;
    bEnableHuntingBehavior = true;
    bEnableMigrationBehavior = false;
    
    PackFormationDistance = 300.0f;
    TerritoryRadius = 1000.0f;
    HuntingRange = 2000.0f;
}

void UDinosaurArchitecture::InitializeComponent()
{
    Super::InitializeComponent();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Architecture System initialized"));
    
    // Initialize dinosaur registry
    InitializeDinosaurRegistry();
    
    // Setup behavior systems
    InitializeBehaviorSystems();
}

void UDinosaurArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate existing dinosaurs in the world
    ValidateExistingDinosaurs();
    
    // Start monitoring systems
    if (bAutoSpawnDinosaurs)
    {
        StartDinosaurSpawning();
    }
}

void UDinosaurArchitecture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update dinosaur monitoring
    UpdateDinosaurMonitoring(DeltaTime);
    
    // Update behavior systems
    UpdateBehaviorSystems(DeltaTime);
}

void UDinosaurArchitecture::InitializeDinosaurRegistry()
{
    DinosaurRegistry.Empty();
    
    // Register T-Rex species
    FEng_DinosaurSpecies TRexSpecies;
    TRexSpecies.SpeciesName = TEXT("Tyrannosaurus Rex");
    TRexSpecies.SpeciesType = EEng_DinosaurType::Carnivore;
    TRexSpecies.MaxCount = MaxTRexCount;
    TRexSpecies.BaseHealth = 1000.0f;
    TRexSpecies.BaseDamage = 200.0f;
    TRexSpecies.MovementSpeed = 600.0f;
    TRexSpecies.DetectionRange = 3000.0f;
    TRexSpecies.bIsPackAnimal = false;
    TRexSpecies.bIsTerritorial = true;
    TRexSpecies.PreferredBiome = EEng_BiomeType::Plains;
    DinosaurRegistry.Add(TEXT("TRex"), TRexSpecies);
    
    // Register Velociraptor species
    FEng_DinosaurSpecies VelociraptorSpecies;
    VelociraptorSpecies.SpeciesName = TEXT("Velociraptor");
    VelociraptorSpecies.SpeciesType = EEng_DinosaurType::Carnivore;
    VelociraptorSpecies.MaxCount = MaxVelociraptorCount;
    VelociraptorSpecies.BaseHealth = 150.0f;
    VelociraptorSpecies.BaseDamage = 75.0f;
    VelociraptorSpecies.MovementSpeed = 800.0f;
    VelociraptorSpecies.DetectionRange = 2000.0f;
    VelociraptorSpecies.bIsPackAnimal = true;
    VelociraptorSpecies.bIsTerritorial = false;
    VelociraptorSpecies.PreferredBiome = EEng_BiomeType::Forest;
    DinosaurRegistry.Add(TEXT("Velociraptor"), VelociraptorSpecies);
    
    // Register Triceratops species
    FEng_DinosaurSpecies TriceratopsSpecies;
    TriceratopsSpecies.SpeciesName = TEXT("Triceratops");
    TriceratopsSpecies.SpeciesType = EEng_DinosaurType::Herbivore;
    TriceratopsSpecies.MaxCount = MaxTriceratopsCount;
    TriceratopsSpecies.BaseHealth = 800.0f;
    TriceratopsSpecies.BaseDamage = 120.0f;
    TriceratopsSpecies.MovementSpeed = 400.0f;
    TriceratopsSpecies.DetectionRange = 1500.0f;
    TriceratopsSpecies.bIsPackAnimal = true;
    TriceratopsSpecies.bIsTerritorial = false;
    TriceratopsSpecies.PreferredBiome = EEng_BiomeType::Plains;
    DinosaurRegistry.Add(TEXT("Triceratops"), TriceratopsSpecies);
    
    // Register Brachiosaurus species
    FEng_DinosaurSpecies BrachiosaurusSpecies;
    BrachiosaurusSpecies.SpeciesName = TEXT("Brachiosaurus");
    BrachiosaurusSpecies.SpeciesType = EEng_DinosaurType::Herbivore;
    BrachiosaurusSpecies.MaxCount = MaxBrachiosaurusCount;
    BrachiosaurusSpecies.BaseHealth = 1500.0f;
    BrachiosaurusSpecies.BaseDamage = 300.0f;
    BrachiosaurusSpecies.MovementSpeed = 200.0f;
    BrachiosaurusSpecies.DetectionRange = 2500.0f;
    BrachiosaurusSpecies.bIsPackAnimal = false;
    BrachiosaurusSpecies.bIsTerritorial = true;
    BrachiosaurusSpecies.PreferredBiome = EEng_BiomeType::Forest;
    DinosaurRegistry.Add(TEXT("Brachiosaurus"), BrachiosaurusSpecies);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur registry initialized with %d species"), DinosaurRegistry.Num());
}

void UDinosaurArchitecture::InitializeBehaviorSystems()
{
    // Initialize pack behavior system
    if (bEnablePackBehavior)
    {
        PackBehaviorSystem.Initialize();
        UE_LOG(LogTemp, Log, TEXT("Pack behavior system initialized"));
    }
    
    // Initialize territorial behavior system
    if (bEnableTerritorialBehavior)
    {
        TerritorialBehaviorSystem.Initialize();
        UE_LOG(LogTemp, Log, TEXT("Territorial behavior system initialized"));
    }
    
    // Initialize hunting behavior system
    if (bEnableHuntingBehavior)
    {
        HuntingBehaviorSystem.Initialize();
        UE_LOG(LogTemp, Log, TEXT("Hunting behavior system initialized"));
    }
}

void UDinosaurArchitecture::ValidateExistingDinosaurs()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count existing dinosaurs by type
    TMap<FString, int32> ExistingCounts;
    
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && IsDinosaurPawn(Pawn))
        {
            FString DinosaurType = GetDinosaurType(Pawn);
            int32* Count = ExistingCounts.Find(DinosaurType);
            if (Count)
            {
                (*Count)++;
            }
            else
            {
                ExistingCounts.Add(DinosaurType, 1);
            }
        }
    }
    
    // Log existing dinosaur counts
    int32 TotalDinosaurs = 0;
    for (const auto& CountPair : ExistingCounts)
    {
        TotalDinosaurs += CountPair.Value;
        UE_LOG(LogTemp, Log, TEXT("Existing %s count: %d"), *CountPair.Key, CountPair.Value);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total existing dinosaurs: %d (limit: %d)"), TotalDinosaurs, MaxTotalDinosaurs);
    
    // Validate against limits
    if (TotalDinosaurs > MaxTotalDinosaurs)
    {
        UE_LOG(LogTemp, Error, TEXT("Dinosaur count exceeds maximum limit!"));
        // Could implement cleanup logic here
    }
}

bool UDinosaurArchitecture::IsDinosaurPawn(APawn* Pawn) const
{
    if (!Pawn)
    {
        return false;
    }
    
    FString ActorName = Pawn->GetName().ToLower();
    
    // Check for dinosaur keywords in actor name
    return ActorName.Contains(TEXT("trex")) ||
           ActorName.Contains(TEXT("veloci")) ||
           ActorName.Contains(TEXT("tricera")) ||
           ActorName.Contains(TEXT("brachi")) ||
           ActorName.Contains(TEXT("ankylo")) ||
           ActorName.Contains(TEXT("parasauro")) ||
           ActorName.Contains(TEXT("dinosaur"));
}

FString UDinosaurArchitecture::GetDinosaurType(APawn* Pawn) const
{
    if (!Pawn)
    {
        return TEXT("Unknown");
    }
    
    FString ActorName = Pawn->GetName().ToLower();
    
    if (ActorName.Contains(TEXT("trex")))
    {
        return TEXT("TRex");
    }
    else if (ActorName.Contains(TEXT("veloci")))
    {
        return TEXT("Velociraptor");
    }
    else if (ActorName.Contains(TEXT("tricera")))
    {
        return TEXT("Triceratops");
    }
    else if (ActorName.Contains(TEXT("brachi")))
    {
        return TEXT("Brachiosaurus");
    }
    else if (ActorName.Contains(TEXT("ankylo")))
    {
        return TEXT("Ankylosaurus");
    }
    else if (ActorName.Contains(TEXT("parasauro")))
    {
        return TEXT("Parasaurolophus");
    }
    
    return TEXT("Other");
}

void UDinosaurArchitecture::StartDinosaurSpawning()
{
    UE_LOG(LogTemp, Log, TEXT("Starting dinosaur spawning system"));
    bDinosaurSpawningActive = true;
}

void UDinosaurArchitecture::StopDinosaurSpawning()
{
    UE_LOG(LogTemp, Log, TEXT("Stopping dinosaur spawning system"));
    bDinosaurSpawningActive = false;
}

void UDinosaurArchitecture::UpdateDinosaurMonitoring(float DeltaTime)
{
    MonitoringTimer += DeltaTime;
    
    // Update monitoring every 5 seconds
    if (MonitoringTimer >= 5.0f)
    {
        MonitoringTimer = 0.0f;
        
        // Re-validate dinosaur counts
        ValidateExistingDinosaurs();
    }
}

void UDinosaurArchitecture::UpdateBehaviorSystems(float DeltaTime)
{
    // Update pack behavior
    if (bEnablePackBehavior)
    {
        PackBehaviorSystem.Update(DeltaTime);
    }
    
    // Update territorial behavior
    if (bEnableTerritorialBehavior)
    {
        TerritorialBehaviorSystem.Update(DeltaTime);
    }
    
    // Update hunting behavior
    if (bEnableHuntingBehavior)
    {
        HuntingBehaviorSystem.Update(DeltaTime);
    }
}

bool UDinosaurArchitecture::CanSpawnDinosaur(const FString& DinosaurType) const
{
    // Check total limit first
    if (GetTotalDinosaurCount() >= MaxTotalDinosaurs)
    {
        return false;
    }
    
    // Check species-specific limits
    const FEng_DinosaurSpecies* Species = DinosaurRegistry.Find(DinosaurType);
    if (Species)
    {
        int32 CurrentCount = GetDinosaurCount(DinosaurType);
        return CurrentCount < Species->MaxCount;
    }
    
    return false;
}

int32 UDinosaurArchitecture::GetTotalDinosaurCount() const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && IsDinosaurPawn(Pawn))
        {
            Count++;
        }
    }
    
    return Count;
}

int32 UDinosaurArchitecture::GetDinosaurCount(const FString& DinosaurType) const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && IsDinosaurPawn(Pawn) && GetDinosaurType(Pawn) == DinosaurType)
        {
            Count++;
        }
    }
    
    return Count;
}

TArray<FString> UDinosaurArchitecture::GetRegisteredSpecies() const
{
    TArray<FString> Species;
    DinosaurRegistry.GetKeys(Species);
    return Species;
}

FEng_DinosaurSpecies UDinosaurArchitecture::GetSpeciesData(const FString& SpeciesName) const
{
    const FEng_DinosaurSpecies* Species = DinosaurRegistry.Find(SpeciesName);
    if (Species)
    {
        return *Species;
    }
    
    return FEng_DinosaurSpecies();
}