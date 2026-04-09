#include "DinosaurVFXComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UDinosaurVFXComponent::UDinosaurVFXComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
}

void UDinosaurVFXComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get VFX Manager reference
    if (UWorld* World = GetWorld())
    {
        VFXManager = World->GetSubsystem<UVFXManager>();
    }
    
    InitializeVFXComponents();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXComponent: Initialized for %s dinosaur"), *DinosaurType);
}

void UDinosaurVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupVFXComponents();
    Super::EndPlay(EndPlayReason);
}

void UDinosaurVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateVFXBasedOnState();
}

void UDinosaurVFXComponent::SetDinosaurVFXState(EDinosaurVFXState NewState)
{
    if (CurrentVFXState == NewState)
    {
        return;
    }
    
    EDinosaurVFXState PreviousState = CurrentVFXState;
    CurrentVFXState = NewState;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXComponent: State changed from %d to %d"), (int32)PreviousState, (int32)NewState);
    
    // Handle state-specific VFX changes
    switch (NewState)
    {
        case EDinosaurVFXState::Moving:
            StartMovementTrail();
            break;
            
        case EDinosaurVFXState::Attacking:
            StopMovementTrail();
            break;
            
        case EDinosaurVFXState::Roaring:
            TriggerRoarBreath(2.0f);
            break;
            
        case EDinosaurVFXState::Injured:
            // Injury effects are handled separately
            break;
            
        case EDinosaurVFXState::Dying:
            StopAllActiveVFX();
            break;
            
        case EDinosaurVFXState::Idle:
        default:
            StopMovementTrail();
            break;
    }
}

void UDinosaurVFXComponent::StartBreathEffect(const FString& BreathType)
{
    if (!VFXSettings.bEnableBreathVFX || !VFXManager)
    {
        return;
    }
    
    StopBreathEffect();
    
    if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        BreathVFXComponent = VFXManager->SpawnVFXAttached(
            EVFXType::DinosaurBreath,
            SkeletalMesh,
            MouthSocketName
        );
        
        if (BreathVFXComponent)
        {
            BreathVFXComponent->SetNiagaraVariableFloat(FString("BreathIntensity"), VFXSettings.VFXIntensityMultiplier);
            BreathVFXComponent->SetNiagaraVariableFloat(FString("DinosaurSize"), DinosaurSizeMultiplier);
            BreathVFXComponent->SetNiagaraVariableLinearColor(FString("BreathColor"), VFXSettings.DinosaurColorTint);
            
            ActiveVFXComponents.Add(BreathVFXComponent);
        }
    }
}

void UDinosaurVFXComponent::StopBreathEffect()
{
    if (BreathVFXComponent)
    {
        VFXManager->StopVFXEffect(BreathVFXComponent);
        ActiveVFXComponents.Remove(BreathVFXComponent);
        BreathVFXComponent = nullptr;
    }
}

void UDinosaurVFXComponent::TriggerRoarBreath(float Duration)
{
    if (!VFXSettings.bEnableRoarVFX)
    {
        return;
    }
    
    StartBreathEffect("Roar");
    
    // Set timer to stop roar breath
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BreathEffectTimer,
            this,
            &UDinosaurVFXComponent::StopBreathEffect,
            Duration,
            false
        );
    }
    
    // Trigger additional roar VFX
    TriggerRoarEffect(2.0f);
}

void UDinosaurVFXComponent::TriggerFootstepEffect(const FVector& FootLocation, const FString& SurfaceType)
{
    if (!VFXSettings.bEnableFootstepVFX || !VFXManager)
    {
        return;
    }
    
    // Spawn dust/debris effect at foot location
    UNiagaraComponent* FootstepVFX = VFXManager->SpawnVFXAtLocation(
        EVFXType::ImpactEffect,
        FootLocation,
        FRotator::ZeroRotator,
        FVector(DinosaurSizeMultiplier * 0.5f)
    );
    
    if (FootstepVFX)
    {
        FootstepVFX->SetNiagaraVariableFloat(FString("SurfaceType"), SurfaceType == "Water" ? 1.0f : 0.0f);
        FootstepVFX->SetNiagaraVariableFloat(FString("FootstepIntensity"), DinosaurSizeMultiplier);
    }
}

void UDinosaurVFXComponent::StartMovementTrail()
{
    if (!VFXManager || MovementTrailComponent)
    {
        return;
    }
    
    MovementTrailComponent = VFXManager->SpawnVFXAttached(
        EVFXType::SmokeTrail,
        GetOwner()->GetRootComponent()
    );
    
    if (MovementTrailComponent)
    {
        MovementTrailComponent->SetNiagaraVariableFloat(FString("TrailIntensity"), DinosaurSizeMultiplier * 0.3f);
        ActiveVFXComponents.Add(MovementTrailComponent);
    }
}

void UDinosaurVFXComponent::StopMovementTrail()
{
    if (MovementTrailComponent)
    {
        VFXManager->StopVFXEffect(MovementTrailComponent);
        ActiveVFXComponents.Remove(MovementTrailComponent);
        MovementTrailComponent = nullptr;
    }
}

void UDinosaurVFXComponent::TriggerAttackEffect(const FVector& AttackLocation, const FVector& AttackDirection)
{
    if (!VFXManager)
    {
        return;
    }
    
    // Spawn attack impact VFX
    UNiagaraComponent* AttackVFX = VFXManager->SpawnVFXAtLocation(
        EVFXType::ImpactEffect,
        AttackLocation,
        AttackDirection.Rotation(),
        FVector(DinosaurSizeMultiplier)
    );
    
    if (AttackVFX)
    {
        AttackVFX->SetNiagaraVariableVec3(FString("AttackDirection"), AttackDirection);
        AttackVFX->SetNiagaraVariableFloat(FString("AttackPower"), DinosaurSizeMultiplier * 2.0f);
    }
    
    // Add screen shake effect through VFX manager
    VFXManager->SetGlobalVFXParameter(FName("ScreenShakeIntensity"), DinosaurSizeMultiplier * 0.5f);
}

void UDinosaurVFXComponent::TriggerInjuryEffect(const FVector& InjuryLocation, float Severity)
{
    if (!VFXSettings.bEnableBloodVFX || !VFXManager)
    {
        return;
    }
    
    VFXManager->SpawnBloodEffect(InjuryLocation, FVector::UpVector, Severity);
    
    // Start bleeding effect if severe injury
    if (Severity > 0.7f)
    {
        StartBleedingEffect(InjuryLocation);
    }
}

void UDinosaurVFXComponent::StartBleedingEffect(const FVector& BleedLocation)
{
    if (BleedingVFXComponent)
    {
        return; // Already bleeding
    }
    
    BleedingVFXComponent = VFXManager->SpawnVFXAtLocation(
        EVFXType::BloodSplatter,
        BleedLocation
    );
    
    if (BleedingVFXComponent)
    {
        BleedingVFXComponent->SetNiagaraVariableFloat(FString("BleedingRate"), 0.5f);
        ActiveVFXComponents.Add(BleedingVFXComponent);
        
        // Set timer to stop bleeding after some time
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                BleedingEffectTimer,
                this,
                &UDinosaurVFXComponent::StopBleedingEffect,
                10.0f,
                false
            );
        }
    }
}

void UDinosaurVFXComponent::StopBleedingEffect()
{
    if (BleedingVFXComponent)
    {
        VFXManager->StopVFXEffect(BleedingVFXComponent);
        ActiveVFXComponents.Remove(BleedingVFXComponent);
        BleedingVFXComponent = nullptr;
    }
}

void UDinosaurVFXComponent::TriggerRoarEffect(float Intensity)
{
    if (!VFXManager)
    {
        return;
    }
    
    FVector RoarLocation = GetSocketLocation(MouthSocketName);
    
    // Spawn roar VFX
    UNiagaraComponent* RoarVFX = VFXManager->SpawnVFXAtLocation(
        EVFXType::ImpactEffect, // Reuse impact for roar shockwave
        RoarLocation,
        FRotator::ZeroRotator,
        FVector(Intensity * DinosaurSizeMultiplier)
    );
    
    if (RoarVFX)
    {
        RoarVFX->SetNiagaraVariableFloat(FString("RoarIntensity"), Intensity);
        RoarVFX->SetNiagaraVariableLinearColor(FString("RoarColor"), VFXSettings.DinosaurColorTint);
    }
    
    // Trigger environmental effects
    StartDustCloud();
}

void UDinosaurVFXComponent::StartFeedingEffect()
{
    // Feeding creates blood and gore effects
    if (VFXSettings.bEnableBloodVFX && VFXManager)
    {
        FVector FeedingLocation = GetSocketLocation(MouthSocketName);
        VFXManager->SpawnBloodEffect(FeedingLocation, FVector::DownVector, 0.8f);
    }
}

void UDinosaurVFXComponent::StopFeedingEffect()
{
    // Stop any feeding-related VFX
    StopBleedingEffect();
}

void UDinosaurVFXComponent::TriggerWaterSplash(const FVector& WaterLocation, float SplashSize)
{
    if (!VFXManager)
    {
        return;
    }
    
    UNiagaraComponent* SplashVFX = VFXManager->SpawnVFXAtLocation(
        EVFXType::WaterSplash,
        WaterLocation,
        FRotator::ZeroRotator,
        FVector(SplashSize * DinosaurSizeMultiplier)
    );
    
    if (SplashVFX)
    {
        SplashVFX->SetNiagaraVariableFloat(FString("SplashIntensity"), DinosaurSizeMultiplier);
    }
}

void UDinosaurVFXComponent::StartDustCloud()
{
    if (DustCloudComponent)
    {
        return;
    }
    
    DustCloudComponent = VFXManager->SpawnVFXAttached(
        EVFXType::SmokeTrail,
        GetOwner()->GetRootComponent(),
        NAME_None,
        FVector(0, 0, -50) // Slightly below the dinosaur
    );
    
    if (DustCloudComponent)
    {
        DustCloudComponent->SetNiagaraVariableFloat(FString("DustIntensity"), DinosaurSizeMultiplier * 0.5f);
        ActiveVFXComponents.Add(DustCloudComponent);
    }
}

void UDinosaurVFXComponent::StopDustCloud()
{
    if (DustCloudComponent)
    {
        VFXManager->StopVFXEffect(DustCloudComponent);
        ActiveVFXComponents.Remove(DustCloudComponent);
        DustCloudComponent = nullptr;
    }
}

void UDinosaurVFXComponent::UpdateVFXSettings(const FDinosaurVFXSettings& NewSettings)
{
    VFXSettings = NewSettings;
    
    // Update existing VFX with new settings
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->SetNiagaraVariableFloat(FString("IntensityMultiplier"), VFXSettings.VFXIntensityMultiplier);
            VFXComp->SetNiagaraVariableLinearColor(FString("ColorTint"), VFXSettings.DinosaurColorTint);
        }
    }
}

void UDinosaurVFXComponent::SetDinosaurSize(float SizeMultiplier)
{
    DinosaurSizeMultiplier = FMath::Clamp(SizeMultiplier, 0.1f, 10.0f);
    
    // Update existing VFX with new size
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->SetNiagaraVariableFloat(FString("SizeMultiplier"), DinosaurSizeMultiplier);
        }
    }
}

void UDinosaurVFXComponent::SetDinosaurType(const FString& NewDinosaurType)
{
    DinosaurType = NewDinosaurType;
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXComponent: Set dinosaur type to %s"), *DinosaurType);
}

void UDinosaurVFXComponent::InitializeVFXComponents()
{
    // Pre-initialize commonly used VFX components for better performance
    if (VFXSettings.bEnableBreathVFX)
    {
        // Breath VFX will be created on-demand
    }
}

void UDinosaurVFXComponent::CleanupVFXComponents()
{
    // Stop all active VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp) && VFXManager)
        {
            VFXManager->StopVFXEffect(VFXComp);
        }
    }
    
    ActiveVFXComponents.Empty();
    
    // Clear component references
    BreathVFXComponent = nullptr;
    MovementTrailComponent = nullptr;
    BleedingVFXComponent = nullptr;
    DustCloudComponent = nullptr;
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BreathEffectTimer);
        World->GetTimerManager().ClearTimer(BleedingEffectTimer);
    }
}

void UDinosaurVFXComponent::UpdateVFXBasedOnState()
{
    // Update VFX parameters based on current state and dinosaur behavior
    switch (CurrentVFXState)
    {
        case EDinosaurVFXState::Moving:
            // Adjust movement trail intensity based on speed
            if (MovementTrailComponent)
            {
                float Speed = GetOwner()->GetVelocity().Size();
                float TrailIntensity = FMath::Clamp(Speed / 1000.0f, 0.1f, 2.0f);
                MovementTrailComponent->SetNiagaraVariableFloat(FString("TrailIntensity"), TrailIntensity);
            }
            break;
            
        case EDinosaurVFXState::Attacking:
            // Increase VFX intensity during attacks
            VFXManager->SetGlobalVFXParameter(FName("CombatIntensity"), 1.5f);
            break;
            
        default:
            // Reset combat intensity
            if (VFXManager)
            {
                VFXManager->SetGlobalVFXParameter(FName("CombatIntensity"), 1.0f);
            }
            break;
    }
}

FVector UDinosaurVFXComponent::GetSocketLocation(const FName& SocketName) const
{
    if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (SkeletalMesh->DoesSocketExist(SocketName))
        {
            return SkeletalMesh->GetSocketLocation(SocketName);
        }
    }
    
    // Fallback to actor location
    return GetOwner()->GetActorLocation();
}

void UDinosaurVFXComponent::StopAllActiveVFX()
{
    StopBreathEffect();
    StopMovementTrail();
    StopBleedingEffect();
    StopDustCloud();
    
    // Stop any remaining active VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp) && VFXManager)
        {
            VFXManager->StopVFXEffect(VFXComp);
        }
    }
    
    ActiveVFXComponents.Empty();
}