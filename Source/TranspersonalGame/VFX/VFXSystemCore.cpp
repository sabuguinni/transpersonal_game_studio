#include "VFXSystemCore.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemCore::UVFXSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Optimize tick rate
    
    // Initialize default performance settings
    PerformanceSettings.MaxParticles = 1000;
    PerformanceSettings.CullDistance = 5000.0f;
    PerformanceSettings.LODLevel = EVFXLODLevel::High;
    PerformanceSettings.bUseGPUSimulation = true;
    PerformanceSettings.bCastShadows = false;
}

void UVFXSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize VFX asset registry
    // These will be populated by the VFX library we'll create
}

void UVFXSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup finished effects
    CleanupFinishedVFX();
    
    // Optimize performance based on current load
    OptimizePerformance();
    
    // Update LOD based on player distance
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
            UpdateLOD(Distance);
        }
    }
}

UNiagaraComponent* UVFXSystemCore::SpawnVFX(EVFXCategory Category, FVector Location, FRotator Rotation, float Scale)
{
    UNiagaraSystem* VFXAsset = GetVFXAsset(Category);
    if (!VFXAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Asset not found for category: %d"), (int32)Category);
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXAsset,
        Location,
        Rotation,
        FVector(Scale),
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (VFXComponent)
    {
        // Apply performance settings
        ApplyLODSettings(VFXComponent, PerformanceSettings.LODLevel);
        
        // Track active VFX
        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location: %s"), 
               *VFXAsset->GetName(), 
               *Location.ToString());
    }
    
    return VFXComponent;
}

void UVFXSystemCore::StopVFX(UNiagaraComponent* VFXComponent, bool bImmediate)
{
    if (!VFXComponent)
        return;
    
    if (bImmediate)
    {
        VFXComponent->DeactivateImmediate();
    }
    else
    {
        VFXComponent->Deactivate();
    }
    
    // Remove from active tracking
    ActiveVFXComponents.Remove(VFXComponent);
}

void UVFXSystemCore::SetVFXIntensity(UNiagaraComponent* VFXComponent, EVFXIntensity Intensity)
{
    if (!VFXComponent)
        return;
    
    float IntensityValue = 1.0f;
    switch (Intensity)
    {
        case EVFXIntensity::Subtle:
            IntensityValue = 0.3f;
            break;
        case EVFXIntensity::Moderate:
            IntensityValue = 0.6f;
            break;
        case EVFXIntensity::Intense:
            IntensityValue = 1.0f;
            break;
        case EVFXIntensity::Extreme:
            IntensityValue = 1.5f;
            break;
    }
    
    // Set Niagara parameters for intensity
    VFXComponent->SetFloatParameter(TEXT("Intensity"), IntensityValue);
    VFXComponent->SetFloatParameter(TEXT("SpawnRate"), IntensityValue * 100.0f);
}

void UVFXSystemCore::ApplyEmotionalProfile(UNiagaraComponent* VFXComponent, const FVFXEmotionalProfile& Profile)
{
    if (!VFXComponent)
        return;
    
    // Apply emotional parameters to Niagara system
    VFXComponent->SetFloatParameter(TEXT("TensionLevel"), Profile.TensionLevel);
    VFXComponent->SetFloatParameter(TEXT("FearFactor"), Profile.FearFactor);
    VFXComponent->SetFloatParameter(TEXT("BeautyFactor"), Profile.BeautyFactor);
    VFXComponent->SetFloatParameter(TEXT("UrgencyLevel"), Profile.UrgencyLevel);
    
    // Adjust color based on emotional state
    FLinearColor EmotionalColor = FLinearColor::White;
    
    if (Profile.FearFactor > 0.7f)
    {
        // High fear = darker, more ominous colors
        EmotionalColor = FLinearColor(0.8f, 0.3f, 0.2f, 1.0f);
    }
    else if (Profile.TensionLevel > 0.7f)
    {
        // High tension = sharper, more contrasted effects
        EmotionalColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
    }
    else if (Profile.BeautyFactor > 0.7f)
    {
        // High beauty = warmer, more natural colors
        EmotionalColor = FLinearColor(0.9f, 0.9f, 0.7f, 1.0f);
    }
    
    VFXComponent->SetColorParameter(TEXT("EmotionalTint"), EmotionalColor);
}

void UVFXSystemCore::SetPerformanceSettings(const FVFXPerformanceSettings& Settings)
{
    PerformanceSettings = Settings;
    
    // Apply to all active VFX
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (VFXComponent)
        {
            ApplyLODSettings(VFXComponent, Settings.LODLevel);
        }
    }
}

void UVFXSystemCore::UpdateLOD(float DistanceToPlayer)
{
    EVFXLODLevel NewLODLevel = EVFXLODLevel::High;
    
    if (DistanceToPlayer > 2000.0f)
    {
        NewLODLevel = EVFXLODLevel::Low;
    }
    else if (DistanceToPlayer > 1000.0f)
    {
        NewLODLevel = EVFXLODLevel::Medium;
    }
    
    if (NewLODLevel != PerformanceSettings.LODLevel)
    {
        PerformanceSettings.LODLevel = NewLODLevel;
        
        // Apply to all active VFX
        for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
        {
            if (VFXComponent)
            {
                ApplyLODSettings(VFXComponent, NewLODLevel);
            }
        }
    }
}

void UVFXSystemCore::TriggerFireEffect(FVector Location, float Intensity, float Duration)
{
    UNiagaraComponent* FireVFX = SpawnVFX(EVFXCategory::Fire, Location);
    if (FireVFX)
    {
        FireVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
        FireVFX->SetFloatParameter(TEXT("Duration"), Duration);
        FireVFX->SetFloatParameter(TEXT("Temperature"), 800.0f + (Intensity * 400.0f));
        
        // Apply survival emotional profile - fire is both beautiful and dangerous
        FVFXEmotionalProfile FireProfile;
        FireProfile.TensionLevel = 0.6f;
        FireProfile.FearFactor = 0.3f;
        FireProfile.BeautyFactor = 0.8f;
        FireProfile.UrgencyLevel = 0.4f;
        
        ApplyEmotionalProfile(FireVFX, FireProfile);
    }
}

void UVFXSystemCore::TriggerSmokeEffect(FVector Location, FVector WindDirection, float Density)
{
    UNiagaraComponent* SmokeVFX = SpawnVFX(EVFXCategory::Smoke, Location);
    if (SmokeVFX)
    {
        SmokeVFX->SetVectorParameter(TEXT("WindDirection"), WindDirection);
        SmokeVFX->SetFloatParameter(TEXT("Density"), Density);
        SmokeVFX->SetFloatParameter(TEXT("LifeTime"), 15.0f);
        
        // Smoke indicates activity - creates tension
        FVFXEmotionalProfile SmokeProfile;
        SmokeProfile.TensionLevel = 0.7f;
        SmokeProfile.FearFactor = 0.4f;
        SmokeProfile.BeautyFactor = 0.2f;
        SmokeProfile.UrgencyLevel = 0.3f;
        
        ApplyEmotionalProfile(SmokeVFX, SmokeProfile);
    }
}

void UVFXSystemCore::TriggerDinosaurBreath(FVector Location, FVector Direction, float Temperature)
{
    UNiagaraComponent* BreathVFX = SpawnVFX(EVFXCategory::DinosaurBreath, Location);
    if (BreathVFX)
    {
        BreathVFX->SetVectorParameter(TEXT("Direction"), Direction);
        BreathVFX->SetFloatParameter(TEXT("Temperature"), Temperature);
        BreathVFX->SetFloatParameter(TEXT("Humidity"), 0.9f);
        
        // Dinosaur breath is a sign of life - creates fear and awe
        FVFXEmotionalProfile BreathProfile;
        BreathProfile.TensionLevel = 0.8f;
        BreathProfile.FearFactor = 0.7f;
        BreathProfile.BeautyFactor = 0.3f;
        BreathProfile.UrgencyLevel = 0.6f;
        
        ApplyEmotionalProfile(BreathVFX, BreathProfile);
    }
}

void UVFXSystemCore::TriggerImpactEffect(FVector Location, float Force, bool bIsLargeCreature)
{
    UNiagaraComponent* ImpactVFX = SpawnVFX(EVFXCategory::DinosaurImpact, Location);
    if (ImpactVFX)
    {
        float ScaleFactor = bIsLargeCreature ? 2.0f : 1.0f;
        ImpactVFX->SetFloatParameter(TEXT("Force"), Force);
        ImpactVFX->SetFloatParameter(TEXT("Scale"), ScaleFactor);
        ImpactVFX->SetIntParameter(TEXT("ParticleCount"), bIsLargeCreature ? 200 : 100);
        
        // Impact effects create immediate tension and fear
        FVFXEmotionalProfile ImpactProfile;
        ImpactProfile.TensionLevel = 0.9f;
        ImpactProfile.FearFactor = 0.8f;
        ImpactProfile.BeautyFactor = 0.1f;
        ImpactProfile.UrgencyLevel = 0.9f;
        
        ApplyEmotionalProfile(ImpactVFX, ImpactProfile);
    }
}

void UVFXSystemCore::SetAtmosphericTension(float TensionLevel)
{
    CurrentTensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    
    // Apply atmospheric tension to all active effects
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (VFXComponent)
        {
            VFXComponent->SetFloatParameter(TEXT("AtmosphericTension"), CurrentTensionLevel);
        }
    }
}

void UVFXSystemCore::TriggerWeatherTransition(float TransitionSpeed)
{
    // This will be implemented with specific weather VFX
    UE_LOG(LogTemp, Log, TEXT("Weather transition triggered with speed: %f"), TransitionSpeed);
}

void UVFXSystemCore::CleanupFinishedVFX()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* VFXComponent)
    {
        return !VFXComponent || !VFXComponent->IsActive();
    });
}

void UVFXSystemCore::OptimizePerformance()
{
    // If we have too many active VFX, reduce quality
    if (ActiveVFXComponents.Num() > PerformanceSettings.MaxParticles / 100)
    {
        // Reduce LOD for distant effects
        for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
        {
            if (VFXComponent)
            {
                float Distance = FVector::Dist(VFXComponent->GetComponentLocation(), 
                                             GetOwner()->GetActorLocation());
                if (Distance > 1000.0f)
                {
                    ApplyLODSettings(VFXComponent, EVFXLODLevel::Low);
                }
            }
        }
    }
}

UNiagaraSystem* UVFXSystemCore::GetVFXAsset(EVFXCategory Category)
{
    if (UNiagaraSystem** FoundAsset = VFXAssets.Find(Category))
    {
        return *FoundAsset;
    }
    
    return nullptr;
}

void UVFXSystemCore::ApplyLODSettings(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel)
{
    if (!VFXComponent)
        return;
    
    float QualityMultiplier = 1.0f;
    int32 ParticleReduction = 1;
    
    switch (LODLevel)
    {
        case EVFXLODLevel::High:
            QualityMultiplier = 1.0f;
            ParticleReduction = 1;
            break;
        case EVFXLODLevel::Medium:
            QualityMultiplier = 0.7f;
            ParticleReduction = 2;
            break;
        case EVFXLODLevel::Low:
            QualityMultiplier = 0.4f;
            ParticleReduction = 4;
            break;
    }
    
    VFXComponent->SetFloatParameter(TEXT("QualityMultiplier"), QualityMultiplier);
    VFXComponent->SetIntParameter(TEXT("ParticleReduction"), ParticleReduction);
}