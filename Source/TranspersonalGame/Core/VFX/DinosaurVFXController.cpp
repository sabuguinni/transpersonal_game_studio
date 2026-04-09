// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "DinosaurVFXController.h"
#include "VFXSystemManager.h"
#include "VFXComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UDinosaurVFXController::UDinosaurVFXController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    CurrentState = EDinosaurVFXState::Idle;
    PreviousState = EDinosaurVFXState::Idle;
    FootstepTimer = 0.0f;
    BreathingTimer = 0.0f;
    StateChangeTime = 0.0f;
    bInWater = false;
    CurrentMovementSpeed = 0.0f;
    CurrentFootstepIndex = 0;
    VFXScaleMultiplier = 1.0f;
    bAutoDetectMovement = true;
    bAutoDetectWater = true;
}

void UDinosaurVFXController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get VFX Manager
    VFXManager = GetWorld()->GetSubsystem<UVFXSystemManager>();
    if (!VFXManager)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurVFXController: Failed to get VFXSystemManager"));
        return;
    }
    
    // Get or create VFX Component
    VFXComponent = GetOwner()->FindComponentByClass<UVFXComponent>();
    if (!VFXComponent)
    {
        VFXComponent = NewObject<UVFXComponent>(GetOwner());
        GetOwner()->AddInstanceComponent(VFXComponent);
        VFXComponent->RegisterComponent();
    }
    
    // Load default configuration
    LoadDefaultConfig();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXController: Initialized for %s"), *GetOwner()->GetName());
}

void UDinosaurVFXController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Cleanup all active VFX
    for (auto& VFXComponent : ActiveBreathingVFX)
    {
        if (VFXComponent.IsValid())
        {
            VFXManager->DestroyVFX(VFXComponent.Get());
        }
    }
    ActiveBreathingVFX.Empty();
    
    for (auto& VFXComponent : ActiveMovementVFX)
    {
        if (VFXComponent.IsValid())
        {
            VFXManager->DestroyVFX(VFXComponent.Get());
        }
    }
    ActiveMovementVFX.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void UDinosaurVFXController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!VFXManager)
        return;
    
    // Auto-detect movement if enabled
    if (bAutoDetectMovement)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                float NewSpeed = MovementComp->Velocity.Size();
                OnMovementSpeedChanged(NewSpeed);
            }
        }
    }
    
    // Update VFX systems
    UpdateMovementVFX(DeltaTime);
    UpdateBreathingVFX(DeltaTime);
    
    // Clean up expired VFX components
    ActiveBreathingVFX.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& Component) {
        return !Component.IsValid();
    });
    
    ActiveMovementVFX.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& Component) {
        return !Component.IsValid();
    });
}

void UDinosaurVFXController::SetDinosaurState(EDinosaurVFXState NewState)
{
    if (CurrentState == NewState)
        return;
    
    PreviousState = CurrentState;
    CurrentState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Cleanup previous state VFX
    CleanupStateVFX(PreviousState);
    
    // Update VFX based on new state
    UpdateStateBasedVFX();
    
    // Broadcast state change
    OnVFXTriggered.Broadcast(CurrentState, EVFXType::None, GetOwner()->GetActorLocation());
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXController: State changed from %d to %d"), 
           (int32)PreviousState, (int32)CurrentState);
}

void UDinosaurVFXController::OnMovementSpeedChanged(float NewSpeed)
{
    CurrentMovementSpeed = NewSpeed;
    
    // Determine state based on movement speed
    if (bAutoDetectMovement)
    {
        EDinosaurVFXState NewState = DetermineStateFromMovement(NewSpeed);
        if (NewState != CurrentState)
        {
            SetDinosaurState(NewState);
        }
    }
}

void UDinosaurVFXController::TriggerFootstepVFX(FName FootSocket)
{
    if (!VFXManager)
        return;
    
    FVector FootstepLocation;
    
    if (FootSocket != NAME_None)
    {
        FootstepLocation = GetFootstepLocation(FootSocket);
    }
    else
    {
        // Use next foot socket in sequence
        if (CurrentConfig.FootstepSockets.Num() > 0)
        {
            FName SocketName = CurrentConfig.FootstepSockets[CurrentFootstepIndex % CurrentConfig.FootstepSockets.Num()];
            FootstepLocation = GetFootstepLocation(SocketName);
            CurrentFootstepIndex++;
        }
        else
        {
            FootstepLocation = GetOwner()->GetActorLocation();
        }
    }
    
    // Get VFX type for current state
    EVFXType* VFXType = CurrentConfig.MovementVFX.Find(CurrentState);
    if (VFXType && *VFXType != EVFXType::None)
    {
        UNiagaraComponent* FootstepVFX = VFXManager->SpawnVFXAtLocation(*VFXType, FootstepLocation);
        if (FootstepVFX)
        {
            ApplyVFXScale(FootstepVFX);
            
            // Set dust generation based on movement speed
            if (CurrentConfig.bGenerateDust && CurrentMovementSpeed > CurrentConfig.DustGenerationThreshold)
            {
                VFXManager->SetVFXParameter(FootstepVFX, TEXT("DustIntensity"), CurrentMovementSpeed / 1000.0f);
            }
            
            OnVFXTriggered.Broadcast(CurrentState, *VFXType, FootstepLocation);
        }
    }
}

void UDinosaurVFXController::SetFootstepInterval(float NewInterval)
{
    CurrentConfig.FootstepVFXInterval = FMath::Max(0.1f, NewInterval);
}

void UDinosaurVFXController::TriggerAttackVFX(const FString& AttackName, FVector AttackLocation)
{
    if (!VFXManager)
        return;
    
    EVFXType* VFXType = CurrentConfig.AttackVFX.Find(AttackName);
    if (VFXType && *VFXType != EVFXType::None)
    {
        FVector SpawnLocation = (AttackLocation != FVector::ZeroVector) ? AttackLocation : GetOwner()->GetActorLocation();
        
        UNiagaraComponent* AttackVFX = VFXManager->SpawnVFXAtLocation(*VFXType, SpawnLocation);
        if (AttackVFX)
        {
            ApplyVFXScale(AttackVFX);
            OnVFXTriggered.Broadcast(EDinosaurVFXState::Attacking, *VFXType, SpawnLocation);
        }
    }
}

void UDinosaurVFXController::TriggerRoarVFX()
{
    if (!VFXManager || CurrentConfig.RoarVFXType == EVFXType::None)
        return;
    
    FVector RoarLocation = GetFootstepLocation(CurrentConfig.BreathingSocket);
    
    UNiagaraComponent* RoarVFX = VFXManager->SpawnVFXAtLocation(CurrentConfig.RoarVFXType, RoarLocation);
    if (RoarVFX)
    {
        ApplyVFXScale(RoarVFX);
        
        // Scale roar intensity based on dinosaur size
        float IntensityMultiplier = 1.0f;
        switch (CurrentConfig.DinosaurSize)
        {
            case EDinosaurSize::Small: IntensityMultiplier = 0.5f; break;
            case EDinosaurSize::Medium: IntensityMultiplier = 1.0f; break;
            case EDinosaurSize::Large: IntensityMultiplier = 1.5f; break;
            case EDinosaurSize::Massive: IntensityMultiplier = 2.0f; break;
        }
        
        VFXManager->SetVFXParameter(RoarVFX, TEXT("Intensity"), IntensityMultiplier);
        OnVFXTriggered.Broadcast(EDinosaurVFXState::Roaring, CurrentConfig.RoarVFXType, RoarLocation);
    }
}

void UDinosaurVFXController::TriggerBloodVFX(FVector HitLocation, FVector HitNormal)
{
    if (!VFXManager || CurrentConfig.BloodVFXType == EVFXType::None)
        return;
    
    FRotator BloodRotation = FRotationMatrix::MakeFromZ(HitNormal).Rotator();
    
    UNiagaraComponent* BloodVFX = VFXManager->SpawnVFXAtLocation(CurrentConfig.BloodVFXType, HitLocation, BloodRotation);
    if (BloodVFX)
    {
        ApplyVFXScale(BloodVFX);
        VFXManager->SetVFXVectorParameter(BloodVFX, TEXT("HitNormal"), HitNormal);
        OnVFXTriggered.Broadcast(EDinosaurVFXState::Injured, CurrentConfig.BloodVFXType, HitLocation);
    }
}

void UDinosaurVFXController::OnEnterWater()
{
    if (bInWater)
        return;
    
    bInWater = true;
    
    if (!VFXManager || !CurrentConfig.bGenerateWaterRipples)
        return;
    
    FVector WaterLocation = GetOwner()->GetActorLocation();
    UNiagaraComponent* RippleVFX = VFXManager->SpawnVFXAtLocation(EVFXType::WaterRipples, WaterLocation);
    if (RippleVFX)
    {
        ApplyVFXScale(RippleVFX);
        OnVFXTriggered.Broadcast(CurrentState, EVFXType::WaterRipples, WaterLocation);
    }
}

void UDinosaurVFXController::OnExitWater()
{
    if (!bInWater)
        return;
    
    bInWater = false;
    
    // Trigger water dripping VFX
    if (VFXManager)
    {
        FVector DrippingLocation = GetOwner()->GetActorLocation();
        UNiagaraComponent* DrippingVFX = VFXManager->SpawnVFXAttached(EVFXType::WaterRipples, GetOwner());
        if (DrippingVFX)
        {
            ApplyVFXScale(DrippingVFX);
            VFXManager->SetVFXParameter(DrippingVFX, TEXT("DrippingMode"), 1.0f);
        }
    }
}

void UDinosaurVFXController::OnGroundImpact(float ImpactForce, FVector ImpactLocation)
{
    if (!VFXManager)
        return;
    
    // Scale VFX based on impact force and dinosaur size
    float VFXIntensity = ImpactForce / 1000.0f; // Normalize impact force
    
    switch (CurrentConfig.DinosaurSize)
    {
        case EDinosaurSize::Small: VFXIntensity *= 0.3f; break;
        case EDinosaurSize::Medium: VFXIntensity *= 0.6f; break;
        case EDinosaurSize::Large: VFXIntensity *= 1.0f; break;
        case EDinosaurSize::Massive: VFXIntensity *= 1.5f; break;
    }
    
    // Spawn dust/debris VFX
    UNiagaraComponent* ImpactVFX = VFXManager->SpawnVFXAtLocation(EVFXType::Dust, ImpactLocation);
    if (ImpactVFX)
    {
        ApplyVFXScale(ImpactVFX);
        VFXManager->SetVFXParameter(ImpactVFX, TEXT("Intensity"), VFXIntensity);
        OnVFXTriggered.Broadcast(CurrentState, EVFXType::Dust, ImpactLocation);
    }
    
    // Spawn ground crack VFX for large impacts
    if (VFXIntensity > 0.5f)
    {
        UNiagaraComponent* CrackVFX = VFXManager->SpawnVFXAtLocation(EVFXType::GroundCrack, ImpactLocation);
        if (CrackVFX)
        {
            ApplyVFXScale(CrackVFX);
            VFXManager->SetVFXParameter(CrackVFX, TEXT("CrackSize"), VFXIntensity);
        }
    }
}

void UDinosaurVFXController::TriggerDeathSequence()
{
    if (!VFXManager)
        return;
    
    SetDinosaurState(EDinosaurVFXState::Dying);
    
    // Trigger death VFX sequence with delays
    FVector DeathLocation = GetOwner()->GetActorLocation();
    
    for (int32 i = 0; i < CurrentConfig.DeathVFXSequence.Num(); i++)
    {
        EVFXType VFXType = CurrentConfig.DeathVFXSequence[i];
        if (VFXType != EVFXType::None)
        {
            float Delay = CurrentConfig.DeathVFXDelay * i;
            
            // Use timer to delay VFX spawning
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, VFXType, DeathLocation]()
            {
                if (VFXManager)
                {
                    UNiagaraComponent* DeathVFX = VFXManager->SpawnVFXAtLocation(VFXType, DeathLocation);
                    if (DeathVFX)
                    {
                        ApplyVFXScale(DeathVFX);
                        OnVFXTriggered.Broadcast(EDinosaurVFXState::Dying, VFXType, DeathLocation);
                    }
                }
            }, Delay, false);
        }
    }
    
    // Set final state after sequence
    FTimerHandle FinalTimerHandle;
    float TotalDelay = CurrentConfig.DeathVFXDelay * CurrentConfig.DeathVFXSequence.Num();
    GetWorld()->GetTimerManager().SetTimer(FinalTimerHandle, [this]()
    {
        SetDinosaurState(EDinosaurVFXState::Dead);
    }, TotalDelay, false);
}

void UDinosaurVFXController::LoadDinosaurConfig(FName ConfigRowName)
{
    if (!DinosaurVFXConfigTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurVFXController: No config table assigned"));
        return;
    }
    
    FDinosaurVFXConfig* ConfigRow = DinosaurVFXConfigTable->FindRow<FDinosaurVFXConfig>(ConfigRowName, TEXT("DinosaurVFXController"));
    if (ConfigRow)
    {
        CurrentConfig = *ConfigRow;
        UE_LOG(LogTemp, Log, TEXT("DinosaurVFXController: Loaded config for %s"), *ConfigRow->DinosaurName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurVFXController: Config row %s not found"), *ConfigRowName.ToString());
        LoadDefaultConfig();
    }
}

void UDinosaurVFXController::SetDinosaurSize(EDinosaurSize NewSize)
{
    CurrentConfig.DinosaurSize = NewSize;
    
    // Update VFX scale based on size
    switch (NewSize)
    {
        case EDinosaurSize::Small: VFXScaleMultiplier = 0.5f; break;
        case EDinosaurSize::Medium: VFXScaleMultiplier = 1.0f; break;
        case EDinosaurSize::Large: VFXScaleMultiplier = 1.5f; break;
        case EDinosaurSize::Massive: VFXScaleMultiplier = 2.0f; break;
    }
}

void UDinosaurVFXController::UpdateMovementVFX(float DeltaTime)
{
    if (CurrentState == EDinosaurVFXState::Walking || CurrentState == EDinosaurVFXState::Running)
    {
        FootstepTimer += DeltaTime;
        
        if (FootstepTimer >= CurrentConfig.FootstepVFXInterval)
        {
            TriggerFootstepVFX();
            FootstepTimer = 0.0f;
        }
    }
    else
    {
        FootstepTimer = 0.0f;
    }
}

void UDinosaurVFXController::UpdateBreathingVFX(float DeltaTime)
{
    if (CurrentState == EDinosaurVFXState::Dead)
        return;
    
    BreathingTimer += DeltaTime;
    
    if (BreathingTimer >= CurrentConfig.BreathingInterval)
    {
        if (VFXManager && CurrentConfig.BreathingVFXType != EVFXType::None)
        {
            FVector BreathingLocation = GetFootstepLocation(CurrentConfig.BreathingSocket);
            
            UNiagaraComponent* BreathingVFX = VFXManager->SpawnVFXAtLocation(CurrentConfig.BreathingVFXType, BreathingLocation);
            if (BreathingVFX)
            {
                ApplyVFXScale(BreathingVFX);
                
                // Adjust breathing intensity based on state
                float BreathingIntensity = 1.0f;
                if (CurrentState == EDinosaurVFXState::Running)
                    BreathingIntensity = 2.0f;
                else if (CurrentState == EDinosaurVFXState::Sleeping)
                    BreathingIntensity = 0.3f;
                
                VFXManager->SetVFXParameter(BreathingVFX, TEXT("Intensity"), BreathingIntensity);
                ActiveBreathingVFX.Add(BreathingVFX);
            }
        }
        
        BreathingTimer = 0.0f;
    }
}

void UDinosaurVFXController::UpdateStateBasedVFX()
{
    // Trigger VFX based on current state
    EVFXType* StateVFX = CurrentConfig.MovementVFX.Find(CurrentState);
    if (StateVFX && *StateVFX != EVFXType::None)
    {
        // State-specific VFX logic handled in individual trigger functions
    }
}

void UDinosaurVFXController::CleanupStateVFX(EDinosaurVFXState State)
{
    // Clean up VFX specific to the previous state
    for (auto& VFXComponent : ActiveMovementVFX)
    {
        if (VFXComponent.IsValid())
        {
            VFXManager->DestroyVFX(VFXComponent.Get());
        }
    }
    ActiveMovementVFX.Empty();
}

void UDinosaurVFXController::ApplyVFXScale(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent)
    {
        FVector Scale = FVector(VFXScaleMultiplier);
        VFXComponent->SetWorldScale3D(Scale);
    }
}

FVector UDinosaurVFXController::GetFootstepLocation(FName FootSocket) const
{
    if (FootSocket == NAME_None)
        return GetOwner()->GetActorLocation();
    
    // Try to get socket location from skeletal mesh
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            if (MeshComp->DoesSocketExist(FootSocket))
            {
                return MeshComp->GetSocketLocation(FootSocket);
            }
        }
    }
    
    // Fallback to actor location
    return GetOwner()->GetActorLocation();
}

EDinosaurVFXState UDinosaurVFXController::DetermineStateFromMovement(float MovementSpeed) const
{
    if (MovementSpeed < 10.0f)
        return EDinosaurVFXState::Idle;
    else if (MovementSpeed < 300.0f)
        return EDinosaurVFXState::Walking;
    else
        return EDinosaurVFXState::Running;
}

void UDinosaurVFXController::LoadDefaultConfig()
{
    // Set up default configuration
    CurrentConfig.DinosaurName = TEXT("Default Dinosaur");
    CurrentConfig.DinosaurSize = EDinosaurSize::Medium;
    CurrentConfig.bIsCarnivore = true;
    CurrentConfig.bIsAquatic = false;
    CurrentConfig.bIsFlying = false;
    
    // Default movement VFX
    CurrentConfig.MovementVFX.Add(EDinosaurVFXState::Walking, EVFXType::DinosaurFootsteps);
    CurrentConfig.MovementVFX.Add(EDinosaurVFXState::Running, EVFXType::DinosaurFootsteps);
    
    CurrentConfig.FootstepVFXInterval = 0.5f;
    CurrentConfig.FootstepSockets = {TEXT("foot_l"), TEXT("foot_r")};
    
    // Default breathing VFX
    CurrentConfig.BreathingVFXType = EVFXType::DinosaurBreathing;
    CurrentConfig.BreathingInterval = 3.0f;
    CurrentConfig.BreathingSocket = TEXT("head");
    
    // Default combat VFX
    CurrentConfig.AttackVFX.Add(TEXT("Bite"), EVFXType::BiteImpact);
    CurrentConfig.AttackVFX.Add(TEXT("Claw"), EVFXType::ClawSlash);
    CurrentConfig.BloodVFXType = EVFXType::BloodSpray;
    CurrentConfig.RoarVFXType = EVFXType::DinosaurRoar;
    
    // Default environmental VFX
    CurrentConfig.bGenerateDust = true;
    CurrentConfig.DustGenerationThreshold = 100.0f;
    CurrentConfig.bGenerateWaterRipples = true;
    
    // Default death VFX
    CurrentConfig.DeathVFXSequence = {EVFXType::BloodSpray, EVFXType::Dust};
    CurrentConfig.DeathVFXDelay = 0.5f;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurVFXController: Loaded default configuration"));
}