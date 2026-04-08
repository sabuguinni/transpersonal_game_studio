#include "VFXSystemCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFXSystemCore::UVFXSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms for performance
}

void UVFXSystemCore::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXLibrary();
}

void UVFXSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Performance monitoring and optimization
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        PerformanceOptimization();
        CleanupExpiredVFX();
        LastPerformanceCheck = 0.0f;
    }

    // Update LOD based on player distance
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            UpdateLODBasedOnDistance(PlayerPawn->GetActorLocation());
        }
    }
}

void UVFXSystemCore::InitializeVFXLibrary()
{
    // Initialize core VFX definitions
    // These will be loaded from data assets in production
    
    // Dinosaur Breath Effect
    FVFXDefinition DinosaurBreath;
    DinosaurBreath.EffectName = FName("DinosaurBreath");
    DinosaurBreath.Category = EVFXCategory::Atmospheric;
    DinosaurBreath.Intensity = EVFXIntensity::Dramatic;
    DinosaurBreath.MaxDrawDistance = 2000.0f;
    DinosaurBreath.MaxParticleCount = 500;
    DinosaurBreath.Lifetime = 3.0f;
    VFXLibrary.Add(DinosaurBreath.EffectName, DinosaurBreath);

    // Campfire Smoke
    FVFXDefinition CampfireSmoke;
    CampfireSmoke.EffectName = FName("CampfireSmoke");
    CampfireSmoke.Category = EVFXCategory::Survival;
    CampfireSmoke.Intensity = EVFXIntensity::Moderate;
    CampfireSmoke.MaxDrawDistance = 1500.0f;
    CampfireSmoke.MaxParticleCount = 300;
    CampfireSmoke.bAutoDestroy = false; // Continuous effect
    VFXLibrary.Add(CampfireSmoke.EffectName, CampfireSmoke);

    // Footstep Dust
    FVFXDefinition FootstepDust;
    FootstepDust.EffectName = FName("FootstepDust");
    FootstepDust.Category = EVFXCategory::Environmental;
    FootstepDust.Intensity = EVFXIntensity::Subtle;
    FootstepDust.MaxDrawDistance = 500.0f;
    FootstepDust.MaxParticleCount = 100;
    FootstepDust.Lifetime = 2.0f;
    VFXLibrary.Add(FootstepDust.EffectName, FootstepDust);

    // Time Gem Effect
    FVFXDefinition TimeGem;
    TimeGem.EffectName = FName("TimeGem");
    TimeGem.Category = EVFXCategory::Mystical;
    TimeGem.Intensity = EVFXIntensity::Cinematic;
    TimeGem.MaxDrawDistance = 3000.0f;
    TimeGem.MaxParticleCount = 1500;
    TimeGem.bAutoDestroy = false; // Important narrative element
    VFXLibrary.Add(TimeGem.EffectName, TimeGem);

    // Tension Atmosphere
    FVFXDefinition TensionAtmosphere;
    TensionAtmosphere.EffectName = FName("TensionAtmosphere");
    TensionAtmosphere.Category = EVFXCategory::Atmospheric;
    TensionAtmosphere.Intensity = EVFXIntensity::Subtle;
    TensionAtmosphere.MaxDrawDistance = 2000.0f;
    TensionAtmosphere.MaxParticleCount = 200;
    TensionAtmosphere.bAutoDestroy = false; // Ambient effect
    VFXLibrary.Add(TensionAtmosphere.EffectName, TensionAtmosphere);
}

UNiagaraComponent* UVFXSystemCore::SpawnVFX(const FVFXDefinition& VFXDef, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    // Check if we're at max concurrent VFX limit
    if (ActiveVFXComponents.Num() >= MaxConcurrentVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System: Max concurrent VFX limit reached (%d)"), MaxConcurrentVFX);
        return nullptr;
    }

    // Select appropriate LOD system
    UNiagaraSystem* SystemToUse = SelectLODSystem(VFXDef, CurrentLODLevel);
    if (!SystemToUse)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX System: No valid Niagara system found for effect %s"), *VFXDef.EffectName.ToString());
        return nullptr;
    }

    // Spawn the VFX
    UNiagaraComponent* VFXComponent = nullptr;
    
    if (AttachToActor)
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            SystemToUse,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            VFXDef.bAutoDestroy
        );
    }
    else
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SystemToUse,
            Location,
            Rotation,
            FVector::OneVector,
            VFXDef.bAutoDestroy
        );
    }

    if (VFXComponent)
    {
        // Set max particle count for performance
        VFXComponent->SetIntParameter(FName("MaxParticles"), VFXDef.MaxParticleCount);
        
        // Track the VFX
        ActiveVFXComponents.Add(VFXComponent);
        VFXSpawnTimes.Add(VFXComponent, GetWorld()->GetTimeSeconds());

        UE_LOG(LogTemp, Log, TEXT("VFX System: Spawned %s at location %s"), *VFXDef.EffectName.ToString(), *Location.ToString());
    }

    return VFXComponent;
}

void UVFXSystemCore::StopVFX(UNiagaraComponent* VFXComponent, bool bImmediate)
{
    if (!VFXComponent)
        return;

    if (bImmediate)
    {
        VFXComponent->DestroyComponent();
    }
    else
    {
        VFXComponent->Deactivate();
    }

    // Remove from tracking
    ActiveVFXComponents.Remove(VFXComponent);
    VFXSpawnTimes.Remove(VFXComponent);
}

void UVFXSystemCore::SetVFXParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, float Value)
{
    if (VFXComponent)
    {
        VFXComponent->SetFloatParameter(ParameterName, Value);
    }
}

void UVFXSystemCore::SetVFXVectorParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, const FVector& Value)
{
    if (VFXComponent)
    {
        VFXComponent->SetVectorParameter(ParameterName, Value);
    }
}

void UVFXSystemCore::SetVFXColorParameter(UNiagaraComponent* VFXComponent, const FName& ParameterName, const FLinearColor& Value)
{
    if (VFXComponent)
    {
        VFXComponent->SetColorParameter(ParameterName, Value);
    }
}

void UVFXSystemCore::SetLODLevel(EVFXLODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        UE_LOG(LogTemp, Log, TEXT("VFX System: LOD level changed to %d"), (int32)NewLODLevel);
        
        // Optionally restart active effects with new LOD
        // This would be expensive, so we'll let them naturally expire
    }
}

void UVFXSystemCore::UpdateLODBasedOnDistance(const FVector& ViewerLocation)
{
    // Calculate average distance to active VFX
    if (ActiveVFXComponents.Num() == 0)
        return;

    float AverageDistance = 0.0f;
    int32 ValidComponents = 0;

    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (VFXComp && VFXComp->IsValidLowLevel())
        {
            float Distance = FVector::Dist(ViewerLocation, VFXComp->GetComponentLocation());
            AverageDistance += Distance;
            ValidComponents++;
        }
    }

    if (ValidComponents > 0)
    {
        AverageDistance /= ValidComponents;

        // Determine appropriate LOD level
        EVFXLODLevel NewLODLevel = EVFXLODLevel::High;
        
        if (AverageDistance > LODDistanceThresholds[2])
        {
            NewLODLevel = EVFXLODLevel::Low;
        }
        else if (AverageDistance > LODDistanceThresholds[1])
        {
            NewLODLevel = EVFXLODLevel::Medium;
        }

        SetLODLevel(NewLODLevel);
    }
}

float UVFXSystemCore::GetTotalParticleCount() const
{
    float TotalParticles = 0.0f;
    
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (VFXComp && VFXComp->IsValidLowLevel())
        {
            // This would need to be implemented with proper Niagara API calls
            // For now, we'll estimate based on the max particle count
            TotalParticles += 100.0f; // Placeholder
        }
    }
    
    return TotalParticles;
}

void UVFXSystemCore::CleanupExpiredVFX()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<UNiagaraComponent*> ComponentsToRemove;

    for (auto& VFXPair : VFXSpawnTimes)
    {
        UNiagaraComponent* VFXComp = VFXPair.Key;
        float SpawnTime = VFXPair.Value;

        // Check if component is invalid or if it's been alive too long
        if (!VFXComp || !VFXComp->IsValidLowLevel() || !VFXComp->IsActive())
        {
            ComponentsToRemove.Add(VFXComp);
        }
        else
        {
            // Check if we have a lifetime limit for this effect
            FName EffectName = FName("Unknown"); // Would need to track this properly
            if (FVFXDefinition* VFXDef = VFXLibrary.Find(EffectName))
            {
                if (VFXDef->bAutoDestroy && (CurrentTime - SpawnTime) > VFXDef->Lifetime)
                {
                    StopVFX(VFXComp, false);
                    ComponentsToRemove.Add(VFXComp);
                }
            }
        }
    }

    // Remove expired components from tracking
    for (UNiagaraComponent* CompToRemove : ComponentsToRemove)
    {
        ActiveVFXComponents.Remove(CompToRemove);
        VFXSpawnTimes.Remove(CompToRemove);
    }

    if (ComponentsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX System: Cleaned up %d expired VFX components"), ComponentsToRemove.Num());
    }
}

UNiagaraSystem* UVFXSystemCore::SelectLODSystem(const FVFXDefinition& VFXDef, EVFXLODLevel LODLevel)
{
    switch (LODLevel)
    {
        case EVFXLODLevel::High:
            return VFXDef.HighLODSystem.LoadSynchronous();
        case EVFXLODLevel::Medium:
            return VFXDef.MediumLODSystem.LoadSynchronous();
        case EVFXLODLevel::Low:
            return VFXDef.LowLODSystem.LoadSynchronous();
        default:
            return VFXDef.HighLODSystem.LoadSynchronous();
    }
}

void UVFXSystemCore::PerformanceOptimization()
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaTimeSeconds();
    
    if (CurrentFPS < PerformanceThresholdFPS)
    {
        // Automatically reduce LOD level if performance is poor
        if (CurrentLODLevel == EVFXLODLevel::High)
        {
            SetLODLevel(EVFXLODLevel::Medium);
            UE_LOG(LogTemp, Warning, TEXT("VFX System: Performance degraded, switching to Medium LOD"));
        }
        else if (CurrentLODLevel == EVFXLODLevel::Medium)
        {
            SetLODLevel(EVFXLODLevel::Low);
            UE_LOG(LogTemp, Warning, TEXT("VFX System: Performance degraded, switching to Low LOD"));
        }
        
        // Also cull distant VFX
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                TArray<UNiagaraComponent*> ComponentsToCull;
                
                for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
                {
                    if (ShouldCullVFX(VFXComp, PlayerLocation))
                    {
                        ComponentsToCull.Add(VFXComp);
                    }
                }
                
                for (UNiagaraComponent* CompToCull : ComponentsToCull)
                {
                    StopVFX(CompToCull, true);
                }
            }
        }
    }
}

bool UVFXSystemCore::ShouldCullVFX(UNiagaraComponent* VFXComponent, const FVector& ViewerLocation)
{
    if (!VFXComponent)
        return true;

    float Distance = FVector::Dist(ViewerLocation, VFXComponent->GetComponentLocation());
    
    // Cull VFX beyond maximum distance based on current LOD level
    float MaxDistance = LODDistanceThresholds[(int32)CurrentLODLevel];
    
    return Distance > MaxDistance;
}

// Preset Effect Implementations
UNiagaraComponent* UVFXSystemCore::SpawnDinosaurBreathVFX(const FVector& Location, const FRotator& Rotation)
{
    if (FVFXDefinition* VFXDef = VFXLibrary.Find(FName("DinosaurBreath")))
    {
        return SpawnVFX(*VFXDef, Location, Rotation);
    }
    return nullptr;
}

UNiagaraComponent* UVFXSystemCore::SpawnCampfireSmokeVFX(const FVector& Location)
{
    if (FVFXDefinition* VFXDef = VFXLibrary.Find(FName("CampfireSmoke")))
    {
        return SpawnVFX(*VFXDef, Location);
    }
    return nullptr;
}

UNiagaraComponent* UVFXSystemCore::SpawnFootstepDustVFX(const FVector& Location, float Intensity)
{
    if (FVFXDefinition* VFXDef = VFXLibrary.Find(FName("FootstepDust")))
    {
        UNiagaraComponent* VFXComp = SpawnVFX(*VFXDef, Location);
        if (VFXComp)
        {
            SetVFXParameter(VFXComp, FName("Intensity"), Intensity);
        }
        return VFXComp;
    }
    return nullptr;
}

UNiagaraComponent* UVFXSystemCore::SpawnTimeGemVFX(const FVector& Location)
{
    if (FVFXDefinition* VFXDef = VFXLibrary.Find(FName("TimeGem")))
    {
        return SpawnVFX(*VFXDef, Location);
    }
    return nullptr;
}

UNiagaraComponent* UVFXSystemCore::SpawnTensionAtmosphereVFX(const FVector& Location, float Radius)
{
    if (FVFXDefinition* VFXDef = VFXLibrary.Find(FName("TensionAtmosphere")))
    {
        UNiagaraComponent* VFXComp = SpawnVFX(*VFXDef, Location);
        if (VFXComp)
        {
            SetVFXParameter(VFXComp, FName("Radius"), Radius);
        }
        return VFXComp;
    }
    return nullptr;
}