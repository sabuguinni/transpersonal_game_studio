#include "LODOptimizationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Landscape/Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"

// Console variables for LOD optimization
static TAutoConsoleVariable<bool> CVarEnableDynamicLOD(
    TEXT("tp.LOD.EnableDynamic"),
    true,
    TEXT("Enable dynamic LOD optimization based on performance"),
    ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarLODDistanceScale(
    TEXT("tp.LOD.DistanceScale"),
    1.0f,
    TEXT("Global scale factor for LOD distances"),
    ECVF_Scalability
);

static TAutoConsoleVariable<int32> CVarMaxHighDetailDinosaurs(
    TEXT("tp.LOD.MaxHighDetailDinosaurs"),
    50,
    TEXT("Maximum number of dinosaurs allowed at highest LOD"),
    ECVF_Scalability
);

TWeakObjectPtr<ALODOptimizationManager> ALODOptimizationManager::Instance = nullptr;

ULODOptimizationSystem::ULODOptimizationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;  // Update every 100ms
    
    // Initialize frame time history
    FrameTimeHistory.SetNum(30);  // Track last 30 frames
    for (int32 i = 0; i < FrameTimeHistory.Num(); i++)
    {
        FrameTimeHistory[i] = 16.67f;  // Default to 60fps
    }
}

void ULODOptimizationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Initializing LOD optimization for dinosaur survival game"));
    
    // Update tracked components
    UpdateTrackedComponents();
    
    // Apply initial LOD settings
    OptimizeLODsForPerformance();
}

void ULODOptimizationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check if we need to update LODs
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLODUpdateTime >= LODUpdateFrequency)
    {
        LastLODUpdateTime = CurrentTime;
        
        if (bEnableDynamicLOD && CVarEnableDynamicLOD.GetValueOnGameThread())
        {
            if (bUsePerformanceBasedLOD)
            {
                AdjustLODBasedOnPerformance();
            }
            
            OptimizeLODsForPerformance();
        }
    }
}

void ULODOptimizationSystem::OptimizeLODsForPerformance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update component tracking
    UpdateTrackedComponents();
    
    // Get camera location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    
    // Optimize static mesh components
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComponent = WeakComponent.Get())
        {
            AActor* Owner = MeshComponent->GetOwner();
            if (!Owner)
            {
                continue;
            }
            
            float Distance = FVector::Dist(CameraLocation, Owner->GetActorLocation());
            
            // Determine appropriate LOD settings based on actor type
            FLODSettings LODSettings;
            if (Owner->GetName().Contains(TEXT("Dinosaur\")))
            {
                if (Owner->GetName().Contains(TEXT("Small\")))
                {
                    LODSettings = DinosaurLODProfile.SmallDinosaur;
                }
                else if (Owner->GetName().Contains(TEXT("Large\")))
                {
                    LODSettings = DinosaurLODProfile.LargeDinosaur;
                }
                else
                {
                    LODSettings = DinosaurLODProfile.MediumDinosaur;
                }
                
                // Apply mass simulation optimization if enabled
                if (bOptimizeForMassSimulation)
                {
                    LODSettings.LOD0Distance *= MassSimulationLODBias;
                    LODSettings.LOD1Distance *= MassSimulationLODBias;
                    LODSettings.LOD2Distance *= MassSimulationLODBias;
                }
            }
            
            // Apply LOD settings
            ApplyLODSettings(MeshComponent, LODSettings);
            
            // Handle culling
            bool bShouldCull = ShouldCullActor(Owner, Distance, LODSettings);
            Owner->SetActorHiddenInGame(bShouldCull);
        }
    }
    
    // Optimize skeletal mesh components (for animated dinosaurs)
    for (auto& WeakComponent : TrackedSkeletalMeshComponents)
    {
        if (USkeletalMeshComponent* SkeletalMeshComponent = WeakComponent.Get())
        {
            AActor* Owner = SkeletalMeshComponent->GetOwner();
            if (!Owner)
            {
                continue;
            }
            
            float Distance = FVector::Dist(CameraLocation, Owner->GetActorLocation());
            
            // Determine LOD settings for animated dinosaurs
            FLODSettings LODSettings = DinosaurLODProfile.MediumDinosaur;
            if (Owner->GetName().Contains(TEXT("Herd\")))
            {
                LODSettings = DinosaurLODProfile.HerdMember;
            }
            
            // Apply skeletal mesh specific optimizations
            int32 TargetLOD = CalculateAppropriiateLOD(Distance, LODSettings);
            SkeletalMeshComponent->SetForcedLOD(TargetLOD + 1);  // UE5 uses 1-based LOD indexing
            
            // Disable animation updates for distant dinosaurs
            if (Distance > LODSettings.LOD2Distance)
            {
                SkeletalMeshComponent->bPauseAnims = true;
                SkeletalMeshComponent->SetComponentTickEnabled(false);
            }
            else
            {
                SkeletalMeshComponent->bPauseAnims = false;
                SkeletalMeshComponent->SetComponentTickEnabled(true);
            }
            
            // Handle culling
            bool bShouldCull = ShouldCullActor(Owner, Distance, LODSettings);
            Owner->SetActorHiddenInGame(bShouldCull);
        }
    }
}

void ULODOptimizationSystem::SetDynamicLODEnabled(bool bEnabled)
{
    bEnableDynamicLOD = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Dynamic LOD %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void ULODOptimizationSystem::UpdateLODDistances(float PerformanceScale)
{
    // Update LOD distances based on performance scale
    DinosaurLODProfile.SmallDinosaur.PerformanceScaleFactor = PerformanceScale;
    DinosaurLODProfile.MediumDinosaur.PerformanceScaleFactor = PerformanceScale;
    DinosaurLODProfile.LargeDinosaur.PerformanceScaleFactor = PerformanceScale;
    DinosaurLODProfile.HerdMember.PerformanceScaleFactor = PerformanceScale;
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Updated LOD distances with scale factor: %f"), PerformanceScale);
}

void ULODOptimizationSystem::ApplyDinosaurLODProfile(AActor* DinosaurActor, const FLODSettings& LODSettings)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    // Find mesh components on the dinosaur
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    
    DinosaurActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    DinosaurActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    // Apply LOD settings to all mesh components
    for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
    {
        ApplyLODSettings(MeshComponent, LODSettings);
    }
    
    for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
    {
        ApplyLODSettings(SkeletalMeshComponent, LODSettings);
    }
}

void ULODOptimizationSystem::OptimizeHerdLODs(const TArray<AActor*>& HerdMembers)
{
    if (HerdMembers.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Optimizing LODs for herd of %d dinosaurs"), HerdMembers.Num());
    
    // Sort herd members by distance to camera
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    
    TArray<TPair<float, AActor*>> SortedHerdMembers;
    for (AActor* HerdMember : HerdMembers)
    {
        if (HerdMember)
        {
            float Distance = FVector::Dist(CameraLocation, HerdMember->GetActorLocation());
            SortedHerdMembers.Add(TPair<float, AActor*>(Distance, HerdMember));
        }
    }
    
    // Sort by distance (closest first)
    SortedHerdMembers.Sort([](const TPair<float, AActor*>& A, const TPair<float, AActor*>& B) {
        return A.Key < B.Key;
    });
    
    // Apply aggressive LOD optimization to herd members
    int32 HighDetailCount = 0;
    for (const auto& HerdPair : SortedHerdMembers)
    {
        AActor* HerdMember = HerdPair.Value;
        float Distance = HerdPair.Key;
        
        FLODSettings HerdLODSettings = DinosaurLODProfile.HerdMember;
        
        // Only allow a limited number of high-detail herd members
        if (HighDetailCount >= MaxHighDetailDinosaurs)
        {
            // Force lower LOD for performance
            HerdLODSettings.LOD0Distance *= 0.3f;
            HerdLODSettings.LOD1Distance *= 0.5f;
            HerdLODSettings.LOD2Distance *= 0.7f;
        }
        else
        {
            HighDetailCount++;
        }
        
        ApplyDinosaurLODProfile(HerdMember, HerdLODSettings);
    }
}

void ULODOptimizationSystem::SetLODForMassSimulation(bool bEnableMassOptimization)
{
    bOptimizeForMassSimulation = bEnableMassOptimization;
    
    if (bEnableMassOptimization)
    {
        // Increase LOD bias for mass simulation
        MassSimulationLODBias = 2.0f;
        MaxHighDetailDinosaurs = FMath::Min(MaxHighDetailDinosaurs, 25);
    }
    else
    {
        MassSimulationLODBias = 1.0f;
        MaxHighDetailDinosaurs = 50;
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Mass simulation optimization %s"), 
           bEnableMassOptimization ? TEXT("Enabled") : TEXT("Disabled"));
}

float ULODOptimizationSystem::GetCurrentFrameTime() const
{
    return AverageFrameTime;
}

int32 ULODOptimizationSystem::GetVisibleActorCount() const
{
    return TrackedMeshComponents.Num() + TrackedSkeletalMeshComponents.Num();
}

void ULODOptimizationSystem::AdjustLODBasedOnPerformance()
{
    float TargetFrameTime = 1000.0f / PerformanceTargetFPS;  // Convert to milliseconds
    float PerformanceRatio = AverageFrameTime / TargetFrameTime;
    
    if (PerformanceRatio > PerformanceThreshold)
    {
        // Performance is below target, increase LOD bias
        float NewLODScale = FMath::Min(CVarLODDistanceScale.GetValueOnGameThread() * 0.9f, 0.3f);
        CVarLODDistanceScale->Set(NewLODScale);
        
        // Reduce max high detail dinosaurs
        MaxHighDetailDinosaurs = FMath::Max(MaxHighDetailDinosaurs - 5, 10);
        
        UE_LOG(LogTemp, Warning, TEXT("LODOptimizationSystem: Performance below target (%.2fms), adjusting LOD scale to %.2f"), 
               AverageFrameTime, NewLODScale);
    }
    else if (PerformanceRatio < 0.8f)
    {
        // Performance is above target, we can relax LOD restrictions
        float NewLODScale = FMath::Min(CVarLODDistanceScale.GetValueOnGameThread() * 1.05f, 1.0f);
        CVarLODDistanceScale->Set(NewLODScale);
        
        // Increase max high detail dinosaurs
        MaxHighDetailDinosaurs = FMath::Min(MaxHighDetailDinosaurs + 2, CVarMaxHighDetailDinosaurs.GetValueOnGameThread());
    }
}

void ULODOptimizationSystem::OptimizeNaniteLODs()
{
    // Nanite handles LOD automatically, but we can optimize Nanite settings
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComponent = WeakComponent.Get())
        {
            if (ShouldUseNaniteForActor(MeshComponent->GetOwner()))
            {
                ConfigureNaniteSettings(MeshComponent);
            }
        }
    }
}

bool ULODOptimizationSystem::ShouldUseNaniteForActor(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Use Nanite for large, complex dinosaurs and environment objects
    if (Actor->GetName().Contains(TEXT("Large\")) || 
        Actor->GetName().Contains(TEXT("Environment\")) ||
        Actor->GetName().Contains(TEXT("Rock\")) ||
        Actor->GetName().Contains(TEXT("Tree\")))
    {
        return true;
    }
    
    return false;
}

void ULODOptimizationSystem::UpdateTrackedComponents()
{
    if (!GetWorld())
    {
        return;
    }
    
    TrackedMeshComponents.Empty();
    TrackedSkeletalMeshComponents.Empty();
    
    // Find all mesh components in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        // Track static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
        {
            TrackedMeshComponents.Add(MeshComponent);
        }
        
        // Track skeletal mesh components
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
        {
            TrackedSkeletalMeshComponents.Add(SkeletalMeshComponent);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Tracking %d static mesh components and %d skeletal mesh components"), 
           TrackedMeshComponents.Num(), TrackedSkeletalMeshComponents.Num());
}

void ULODOptimizationSystem::ApplyLODSettings(UMeshComponent* MeshComponent, const FLODSettings& Settings)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Calculate distance to camera
    float Distance = CalculateDistanceToCamera(MeshComponent->GetOwner());
    
    // Apply distance scaling
    float ScaledDistance = Distance * CVarLODDistanceScale.GetValueOnGameThread() * Settings.PerformanceScaleFactor;
    
    // Calculate appropriate LOD
    int32 TargetLOD = CalculateAppropriiateLOD(ScaledDistance, Settings);
    
    // Apply LOD to static mesh component
    if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
    {
        StaticMeshComponent->SetForcedLodModel(TargetLOD + 1);  // UE5 uses 1-based LOD indexing
    }
}

float ULODOptimizationSystem::CalculateDistanceToCamera(const AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
}

int32 ULODOptimizationSystem::CalculateAppropriiateLOD(float Distance, const FLODSettings& Settings) const
{
    if (Distance <= Settings.LOD0Distance)
    {
        return 0;
    }
    else if (Distance <= Settings.LOD1Distance)
    {
        return 1;
    }
    else if (Distance <= Settings.LOD2Distance)
    {
        return 2;
    }
    else if (Distance <= Settings.LOD3Distance)
    {
        return 3;
    }
    else
    {
        return 4;  // Highest LOD (lowest detail)
    }
}

void ULODOptimizationSystem::UpdatePerformanceMetrics()
{
    // Get current frame time
    float CurrentFrameTime = FPlatformTime::ToMilliseconds(FPlatformTime::Cycles() - GFrameCounter);
    
    // Update frame time history
    FrameTimeHistory[FrameTimeHistoryIndex] = CurrentFrameTime;
    FrameTimeHistoryIndex = (FrameTimeHistoryIndex + 1) % FrameTimeHistory.Num();
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
}

bool ULODOptimizationSystem::ShouldCullActor(const AActor* Actor, float Distance, const FLODSettings& Settings) const
{
    return Distance > Settings.CullDistance;
}

void ULODOptimizationSystem::EnableNaniteForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
    {
        ConfigureNaniteSettings(MeshComponent);
    }
}

void ULODOptimizationSystem::ConfigureNaniteSettings(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent || !MeshComponent->GetStaticMesh())
    {
        return;
    }
    
    // Configure Nanite settings for optimal performance
    // Note: Nanite settings are typically configured on the static mesh asset itself
    // This is a placeholder for runtime Nanite optimization
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationSystem: Configured Nanite settings for %s"), 
           *MeshComponent->GetOwner()->GetName());
}

// ALODOptimizationManager Implementation

ALODOptimizationManager::ALODOptimizationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;  // Update every 500ms
    
    // Create LOD optimization system component
    LODSystem = CreateDefaultSubobject<ULODOptimizationSystem>(TEXT("LODSystem\"));
}

void ALODOptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set this as the global instance
    Instance = this;
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationManager: Global LOD optimization manager initialized"));
}

void ALODOptimizationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        LastOptimizationTime = CurrentTime;
        OptimizeAllDinosaurs();
    }
}

ALODOptimizationManager* ALODOptimizationManager::GetInstance(UWorld* World)
{
    if (Instance.IsValid())
    {
        return Instance.Get();
    }
    
    // Try to find existing instance
    for (TActorIterator<ALODOptimizationManager> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ALODOptimizationManager* Manager = *ActorIterator;
        if (Manager && !Manager->IsPendingKill())
        {
            Instance = Manager;
            return Manager;
        }
    }
    
    // Create new instance if none found
    ALODOptimizationManager* NewManager = World->SpawnActor<ALODOptimizationManager>();
    Instance = NewManager;
    return NewManager;
}

void ALODOptimizationManager::RegisterDinosaur(AActor* DinosaurActor, const FString& DinosaurType)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    RegisteredDinosaurs.Add(DinosaurActor, DinosaurType);
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationManager: Registered dinosaur %s of type %s"), 
           *DinosaurActor->GetName(), *DinosaurType);
}

void ALODOptimizationManager::UnregisterDinosaur(AActor* DinosaurActor)
{
    if (!DinosaurActor)
    {
        return;
    }
    
    RegisteredDinosaurs.Remove(DinosaurActor);
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationManager: Unregistered dinosaur %s"), *DinosaurActor->GetName());
}

void ALODOptimizationManager::OptimizeAllDinosaurs()
{
    if (!LODSystem)
    {
        return;
    }
    
    // Clean up invalid entries
    for (auto It = RegisteredDinosaurs.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || It.Key()->IsPendingKill())
        {
            It.RemoveCurrent();
        }
    }
    
    // Group dinosaurs by type for optimized processing
    TMap<FString, TArray<AActor*>> DinosaursByType;
    for (const auto& DinosaurPair : RegisteredDinosaurs)
    {
        if (AActor* Dinosaur = DinosaurPair.Key.Get())
        {
            DinosaursByType.FindOrAdd(DinosaurPair.Value).Add(Dinosaur);
        }
    }
    
    // Optimize each group
    for (const auto& TypeGroup : DinosaursByType)
    {
        const FString& DinosaurType = TypeGroup.Key;
        const TArray<AActor*>& Dinosaurs = TypeGroup.Value;
        
        if (DinosaurType.Contains(TEXT("Herd\")))
        {
            LODSystem->OptimizeHerdLODs(Dinosaurs);
        }
        else
        {
            // Apply individual optimization
            for (AActor* Dinosaur : Dinosaurs)
            {
                FLODSettings LODSettings;
                if (DinosaurType.Contains(TEXT("Small\")))
                {
                    LODSettings = LODSystem->DinosaurLODProfile.SmallDinosaur;
                }
                else if (DinosaurType.Contains(TEXT("Large\")))
                {
                    LODSettings = LODSystem->DinosaurLODProfile.LargeDinosaur;
                }
                else
                {
                    LODSettings = LODSystem->DinosaurLODProfile.MediumDinosaur;
                }
                
                LODSystem->ApplyDinosaurLODProfile(Dinosaur, LODSettings);
            }
        }
    }
}

void ALODOptimizationManager::SetGlobalLODScale(float Scale)
{
    GlobalLODScale = FMath::Clamp(Scale, 0.1f, 2.0f);
    CVarLODDistanceScale->Set(GlobalLODScale);
    
    if (LODSystem)
    {
        LODSystem->UpdateLODDistances(GlobalLODScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODOptimizationManager: Set global LOD scale to %f"), GlobalLODScale);
}