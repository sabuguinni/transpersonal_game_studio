#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update VFX LOD 10 times per second
    
    // Initialize VFX pool
    VFXPool.Reserve(MaxActiveVFXSystems);
    ActiveVFXSystems.Reserve(MaxActiveVFXSystems);
    
    ActiveVFXCount = 0;
    PooledVFXCount = 0;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player reference for LOD calculations
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Setup cleanup timer - run every 5 seconds
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimer,
        this,
        &AVFXManager::CleanupExpiredVFX,
        5.0f,
        true
    );
    
    // Pre-populate VFX pool
    for (int32 i = 0; i < 20; ++i)
    {
        UNiagaraComponent* PooledComponent = CreateDefaultSubobject<UNiagaraComponent>(
            *FString::Printf(TEXT("PooledVFX_%d"), i)
        );
        PooledComponent->SetAutoDestroy(false);
        PooledComponent->SetActive(false);
        VFXPool.Add(PooledComponent);
        PooledVFXCount++;
    }
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update LOD for all active VFX systems
    UpdateVFXLOD();
}

UNiagaraComponent* AVFXManager::SpawnVFXAtLocation(
    UNiagaraSystem* VFXSystem,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    bool bAutoDestroy,
    float LifeTime)
{
    if (!VFXSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Attempted to spawn null VFX system"));
        return nullptr;
    }
    
    // Check if we're at the limit
    if (ActiveVFXSystems.Num() >= MaxActiveVFXSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Maximum VFX systems reached, skipping spawn"));
        return nullptr;
    }
    
    // Get component from pool or create new
    UNiagaraComponent* VFXComponent = GetPooledVFXComponent();
    if (!VFXComponent)
    {
        return nullptr;
    }
    
    // Configure the component
    VFXComponent->SetAsset(VFXSystem);
    VFXComponent->SetWorldLocationAndRotation(Location, Rotation);
    VFXComponent->SetWorldScale3D(Scale);
    VFXComponent->SetActive(true);
    VFXComponent->Activate();
    
    // Set initial LOD based on distance
    int32 LODLevel = CalculateVFXLOD(Location);
    VFXComponent->SetIntParameter(TEXT("LOD_Level"), LODLevel);
    
    // Add to active systems
    ActiveVFXSystems.Add(VFXComponent);
    ActiveVFXCount++;
    
    // Setup auto-destroy if requested
    if (bAutoDestroy && LifeTime > 0.0f)
    {
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(
            DestroyTimer,
            [this, VFXComponent]()
            {
                if (VFXComponent && IsValid(VFXComponent))
                {
                    ReturnToPool(VFXComponent);
                }
            },
            LifeTime,
            false
        );
    }
    
    return VFXComponent;
}

UNiagaraComponent* AVFXManager::SpawnVFXAttached(
    UNiagaraSystem* VFXSystem,
    USceneComponent* AttachToComponent,
    FName AttachPointName,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    bool bAutoDestroy,
    float LifeTime)
{
    if (!VFXSystem || !AttachToComponent)
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = SpawnVFXAtLocation(
        VFXSystem, Location, Rotation, Scale, bAutoDestroy, LifeTime
    );
    
    if (VFXComponent)
    {
        VFXComponent->AttachToComponent(
            AttachToComponent,
            FAttachmentTransformRules::KeepRelativeTransform,
            AttachPointName
        );
    }
    
    return VFXComponent;
}

void AVFXManager::SetRainIntensity(float Intensity)
{
    // Broadcast rain intensity to all environmental VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXSystems)
    {
        if (VFXComp && VFXComp->GetAsset())
        {
            FString AssetName = VFXComp->GetAsset()->GetName();
            if (AssetName.Contains(TEXT("Rain")) || AssetName.Contains(TEXT("Weather")))
            {
                VFXComp->SetFloatParameter(TEXT("Rain_Intensity"), Intensity);
            }
        }
    }
}

void AVFXManager::SetWindStrength(float WindStrength, FVector WindDirection)
{
    // Update wind parameters for vegetation and atmospheric effects
    for (UNiagaraComponent* VFXComp : ActiveVFXSystems)
    {
        if (VFXComp && VFXComp->GetAsset())
        {
            FString AssetName = VFXComp->GetAsset()->GetName();
            if (AssetName.Contains(TEXT("Wind")) || AssetName.Contains(TEXT("Leaves")))
            {
                VFXComp->SetFloatParameter(TEXT("Wind_Strength"), WindStrength);
                VFXComp->SetVectorParameter(TEXT("Wind_Direction"), WindDirection);
            }
        }
    }
}

void AVFXManager::SetAtmosphericDust(float Density, FLinearColor Color)
{
    // Update atmospheric dust for depth and mood
    for (UNiagaraComponent* VFXComp : ActiveVFXSystems)
    {
        if (VFXComp && VFXComp->GetAsset())
        {
            FString AssetName = VFXComp->GetAsset()->GetName();
            if (AssetName.Contains(TEXT("Dust")) || AssetName.Contains(TEXT("Atmosphere")))
            {
                VFXComp->SetFloatParameter(TEXT("Dust_Density"), Density);
                VFXComp->SetColorParameter(TEXT("Dust_Color"), Color);
            }
        }
    }
}

UNiagaraComponent* AVFXManager::SpawnDinosaurBreath(
    AActor* DinosaurActor,
    float BreathIntensity,
    FLinearColor BreathColor)
{
    if (!DinosaurActor)
    {
        return nullptr;
    }
    
    // Find mouth/nose attachment point
    USceneComponent* AttachPoint = DinosaurActor->GetRootComponent();
    
    // Load breath VFX system (this would be a real asset reference)
    UNiagaraSystem* BreathSystem = nullptr; // LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Dinosaurs/NS_DinosaurBreath"));
    
    if (!BreathSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Dinosaur breath VFX system not found"));
        return nullptr;
    }
    
    UNiagaraComponent* BreathVFX = SpawnVFXAttached(
        BreathSystem,
        AttachPoint,
        TEXT("MouthSocket"),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector::OneVector,
        false, // Don't auto-destroy, managed by dinosaur
        0.0f
    );
    
    if (BreathVFX)
    {
        BreathVFX->SetFloatParameter(TEXT("Breath_Intensity"), BreathIntensity);
        BreathVFX->SetColorParameter(TEXT("Breath_Color"), BreathColor);
    }
    
    return BreathVFX;
}

void AVFXManager::SpawnFootstepVFX(
    FVector FootLocation,
    float DinosaurWeight,
    int32 SurfaceType)
{
    // Different VFX based on surface type
    UNiagaraSystem* FootstepSystem = nullptr;
    
    switch (SurfaceType)
    {
        case 0: // Mud
            // FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_FootstepMud"));
            break;
        case 1: // Grass
            // FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_FootstepGrass"));
            break;
        case 2: // Stone
            // FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_FootstepStone"));
            break;
        default:
            // FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_FootstepGeneric"));
            break;
    }
    
    if (FootstepSystem)
    {
        UNiagaraComponent* FootstepVFX = SpawnVFXAtLocation(
            FootstepSystem,
            FootLocation,
            FRotator::ZeroRotator,
            FVector::OneVector,
            true,
            3.0f // Short-lived effect
        );
        
        if (FootstepVFX)
        {
            FootstepVFX->SetFloatParameter(TEXT("Weight_Scale"), DinosaurWeight);
            FootstepVFX->SetIntParameter(TEXT("Surface_Type"), SurfaceType);
        }
    }
}

UNiagaraComponent* AVFXManager::SpawnTamingVFX(AActor* DinosaurActor, float TamingProgress)
{
    if (!DinosaurActor)
    {
        return nullptr;
    }
    
    // Load taming VFX system
    UNiagaraSystem* TamingSystem = nullptr; // LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Gameplay/NS_TamingEffect"));
    
    if (!TamingSystem)
    {
        return nullptr;
    }
    
    UNiagaraComponent* TamingVFX = SpawnVFXAttached(
        TamingSystem,
        DinosaurActor->GetRootComponent(),
        NAME_None,
        FVector(0, 0, 100), // Above the dinosaur
        FRotator::ZeroRotator,
        FVector::OneVector,
        false,
        0.0f
    );
    
    if (TamingVFX)
    {
        TamingVFX->SetFloatParameter(TEXT("Taming_Progress"), TamingProgress);
        
        // Color changes based on progress: red -> yellow -> green
        FLinearColor TamingColor = FLinearColor::LerpUsingHSV(
            FLinearColor::Red,
            FLinearColor::Green,
            TamingProgress
        );
        TamingVFX->SetColorParameter(TEXT("Taming_Color"), TamingColor);
    }
    
    return TamingVFX;
}

UNiagaraComponent* AVFXManager::SpawnFireVFX(
    FVector Location,
    float FireIntensity,
    bool bIsTorch)
{
    // Different fire systems for different uses
    UNiagaraSystem* FireSystem = nullptr;
    
    if (bIsTorch)
    {
        // FireSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Survival/NS_TorchFire"));
    }
    else
    {
        // FireSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Survival/NS_CampfireFire"));
    }
    
    if (!FireSystem)
    {
        return nullptr;
    }
    
    UNiagaraComponent* FireVFX = SpawnVFXAtLocation(
        FireSystem,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        false, // Fire persists until manually destroyed
        0.0f
    );
    
    if (FireVFX)
    {
        FireVFX->SetFloatParameter(TEXT("Fire_Intensity"), FireIntensity);
        FireVFX->SetFloatParameter(TEXT("Heat_Distortion"), FireIntensity * 0.5f);
    }
    
    return FireVFX;
}

void AVFXManager::SpawnToolImpactVFX(
    FVector ImpactLocation,
    FVector ImpactNormal,
    int32 MaterialType,
    float ImpactForce)
{
    UNiagaraSystem* ImpactSystem = nullptr;
    
    switch (MaterialType)
    {
        case 0: // Wood
            // ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Tools/NS_WoodImpact"));
            break;
        case 1: // Stone
            // ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Tools/NS_StoneImpact"));
            break;
        case 2: // Metal
            // ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Tools/NS_MetalImpact"));
            break;
        default:
            // ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Tools/NS_GenericImpact"));
            break;
    }
    
    if (ImpactSystem)
    {
        FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
        
        UNiagaraComponent* ImpactVFX = SpawnVFXAtLocation(
            ImpactSystem,
            ImpactLocation,
            ImpactRotation,
            FVector::OneVector,
            true,
            2.0f
        );
        
        if (ImpactVFX)
        {
            ImpactVFX->SetFloatParameter(TEXT("Impact_Force"), ImpactForce);
            ImpactVFX->SetVectorParameter(TEXT("Impact_Normal"), ImpactNormal);
        }
    }
}

void AVFXManager::SpawnWaterVFX(FVector Location, int32 WaterType, float Intensity)
{
    UNiagaraSystem* WaterSystem = nullptr;
    
    switch (WaterType)
    {
        case 0: // Dripping
            // WaterSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_WaterDrip"));
            break;
        case 1: // Splash
            // WaterSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_WaterSplash"));
            break;
        case 2: // Stream
            // WaterSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_WaterStream"));
            break;
    }
    
    if (WaterSystem)
    {
        UNiagaraComponent* WaterVFX = SpawnVFXAtLocation(
            WaterSystem,
            Location,
            FRotator::ZeroRotator,
            FVector::OneVector,
            WaterType != 2, // Streams don't auto-destroy
            WaterType == 1 ? 3.0f : 0.0f // Splashes last 3 seconds
        );
        
        if (WaterVFX)
        {
            WaterVFX->SetFloatParameter(TEXT("Water_Intensity"), Intensity);
        }
    }
}

void AVFXManager::UpdateVFXLOD()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (UNiagaraComponent* VFXComp : ActiveVFXSystems)
    {
        if (!VFXComp || !VFXComp->IsActive())
        {
            continue;
        }
        
        FVector VFXLocation = VFXComp->GetComponentLocation();
        float Distance = FVector::Dist(PlayerLocation, VFXLocation);
        
        int32 NewLOD = CalculateVFXLOD(VFXLocation);
        VFXComp->SetIntParameter(TEXT("LOD_Level"), NewLOD);
        
        // Disable very distant VFX to save performance
        if (Distance > VFXLODDistance_Low * 1.5f)
        {
            VFXComp->SetActive(false);
        }
        else if (!VFXComp->IsActive() && Distance <= VFXLODDistance_Low)
        {
            VFXComp->SetActive(true);
        }
    }
}

void AVFXManager::CleanupExpiredVFX()
{
    ActiveVFXSystems.RemoveAll([this](UNiagaraComponent* VFXComp)
    {
        if (!VFXComp || !IsValid(VFXComp) || (!VFXComp->IsActive() && VFXComp->HasCompleted()))
        {
            if (VFXComp)
            {
                ReturnToPool(VFXComp);
            }
            return true;
        }
        return false;
    });
    
    ActiveVFXCount = ActiveVFXSystems.Num();
}

int32 AVFXManager::CalculateVFXLOD(FVector VFXLocation)
{
    if (!PlayerPawn)
    {
        return 2; // Default to low LOD if no player
    }
    
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), VFXLocation);
    
    if (Distance <= VFXLODDistance_High)
    {
        return 0; // High LOD
    }
    else if (Distance <= VFXLODDistance_Medium)
    {
        return 1; // Medium LOD
    }
    else
    {
        return 2; // Low LOD
    }
}

UNiagaraComponent* AVFXManager::GetPooledVFXComponent()
{
    // Try to get from pool first
    for (int32 i = VFXPool.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* PooledComp = VFXPool[i];
        if (PooledComp && !PooledComp->IsActive())
        {
            VFXPool.RemoveAt(i);
            PooledVFXCount--;
            return PooledComp;
        }
    }
    
    // Create new component if pool is empty
    UNiagaraComponent* NewComponent = CreateDefaultSubobject<UNiagaraComponent>(
        *FString::Printf(TEXT("DynamicVFX_%d"), FMath::Rand())
    );
    NewComponent->SetAutoDestroy(false);
    
    return NewComponent;
}

void AVFXManager::ReturnToPool(UNiagaraComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Remove from active systems
    ActiveVFXSystems.Remove(Component);
    ActiveVFXCount--;
    
    // Reset component state
    Component->SetActive(false);
    Component->SetAsset(nullptr);
    Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    
    // Return to pool if there's space
    if (VFXPool.Num() < MaxActiveVFXSystems / 2)
    {
        VFXPool.Add(Component);
        PooledVFXCount++;
    }
    else
    {
        // Destroy if pool is full
        Component->DestroyComponent();
    }
}