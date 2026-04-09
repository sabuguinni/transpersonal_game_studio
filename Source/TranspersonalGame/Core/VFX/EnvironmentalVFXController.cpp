// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentalVFXController.h"
#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"

UEnvironmentalVFXController::UEnvironmentalVFXController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentWeatherState = EWeatherState::Clear;
    PreviousWeatherState = EWeatherState::Clear;
    CurrentWindSpeed = 0.0f;
    bPlayerInArea = false;
    LastPlayerLocation = FVector::ZeroVector;
    LastPlayerMovementSpeed = 0.0f;
    bRainActive = false;
    CurrentRainIntensity = 0.0f;
    
    // Default configuration
    bAutoDetectWeather = true;
    bAutoDetectPlayerProximity = true;
    PlayerDetectionRadius = 2000.0f;
    GlobalIntensityMultiplier = 1.0f;
    
    // Enable common VFX types by default
    EnabledVFXTypes.Add(EEnvironmentalVFXType::TreeSway);
    EnabledVFXTypes.Add(EEnvironmentalVFXType::LeafFall);
    EnabledVFXTypes.Add(EEnvironmentalVFXType::GrassRustle);
    EnabledVFXTypes.Add(EEnvironmentalVFXType::Dust);
    EnabledVFXTypes.Add(EEnvironmentalVFXType::WaterRipples);
    EnabledVFXTypes.Add(EEnvironmentalVFXType::Fireflies);
}

void UEnvironmentalVFXController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get VFX System Manager
    VFXManager = UVFXSystemManager::GetInstance(GetWorld());
    if (!VFXManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentalVFXController: Could not find VFXSystemManager"));
    }
    
    // Load VFX configurations from data table
    if (EnvironmentalVFXConfigTable)
    {
        TArray<FEnvironmentalVFXConfig*> AllConfigs;
        EnvironmentalVFXConfigTable->GetAllRows<FEnvironmentalVFXConfig>(TEXT("EnvironmentalVFXController"), AllConfigs);
        
        for (FEnvironmentalVFXConfig* Config : AllConfigs)
        {
            if (Config)
            {
                VFXConfigs.Add(Config->VFXType, *Config);
                UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Loaded config for VFX type %d"), (int32)Config->VFXType);
            }
        }
    }
    
    // Initialize VFX timers
    for (const EEnvironmentalVFXType& VFXType : EnabledVFXTypes)
    {
        VFXTimers.Add(VFXType, 0.0f);
        ActiveVFX.Add(VFXType, TArray<TWeakObjectPtr<UNiagaraComponent>>());
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Initialized with %d enabled VFX types"), EnabledVFXTypes.Num());
}

void UEnvironmentalVFXController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all active VFX
    for (auto& VFXPair : ActiveVFX)
    {
        for (TWeakObjectPtr<UNiagaraComponent>& VFXComponent : VFXPair.Value)
        {
            if (VFXComponent.IsValid())
            {
                VFXComponent->DestroyComponent();
            }
        }
    }
    ActiveVFX.Empty();
    
    // Clean up rain VFX
    for (TWeakObjectPtr<UNiagaraComponent>& RainComponent : ActiveRainVFX)
    {
        if (RainComponent.IsValid())
        {
            RainComponent->DestroyComponent();
        }
    }
    ActiveRainVFX.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void UEnvironmentalVFXController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!VFXManager)
    {
        return;
    }
    
    // Update weather-based VFX
    UpdateWeatherVFX(DeltaTime);
    
    // Update player proximity VFX
    if (bAutoDetectPlayerProximity)
    {
        UpdatePlayerProximityVFX(DeltaTime);
    }
    
    // Update auto-trigger VFX
    UpdateAutoTriggerVFX(DeltaTime);
    
    // Clean up expired VFX
    CleanupExpiredVFX();
}

void UEnvironmentalVFXController::SetWeatherState(EWeatherState NewWeatherState)
{
    if (CurrentWeatherState != NewWeatherState)
    {
        PreviousWeatherState = CurrentWeatherState;
        CurrentWeatherState = NewWeatherState;
        
        UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Weather state changed from %d to %d"), 
               (int32)PreviousWeatherState, (int32)CurrentWeatherState);
        
        // Handle weather transition effects
        switch (CurrentWeatherState)
        {
            case EWeatherState::Rainy:
                StartRainEffect(1.0f);
                break;
            case EWeatherState::Clear:
                if (PreviousWeatherState == EWeatherState::Rainy)
                {
                    StopRainEffect();
                }
                break;
            case EWeatherState::Stormy:
                StartRainEffect(1.5f);
                SetWindSpeed(FMath::RandRange(15.0f, 25.0f));
                break;
            case EWeatherState::Windy:
                SetWindSpeed(FMath::RandRange(8.0f, 15.0f));
                break;
            default:
                break;
        }
    }
}

void UEnvironmentalVFXController::SetWindSpeed(float NewWindSpeed)
{
    CurrentWindSpeed = FMath::Clamp(NewWindSpeed, 0.0f, 50.0f);
    
    // Trigger wind-based effects
    if (CurrentWindSpeed > 5.0f)
    {
        TriggerTreeSway(CurrentWindSpeed / 10.0f);
        
        if (CurrentWindSpeed > 10.0f)
        {
            TriggerEnvironmentalVFX(EEnvironmentalVFXType::GrassRustle, FVector::ZeroVector, CurrentWindSpeed / 15.0f);
        }
    }
}

void UEnvironmentalVFXController::TriggerEnvironmentalVFX(EEnvironmentalVFXType VFXType, FVector Location, float Intensity)
{
    if (!CanTriggerVFX(VFXType))
    {
        return;
    }
    
    // Use owner's location if no location specified
    if (Location.IsZero() && GetOwner())
    {
        Location = GetOwner()->GetActorLocation();
    }
    
    // Get configuration for this VFX type
    FEnvironmentalVFXConfig* Config = VFXConfigs.Find(VFXType);
    if (!Config)
    {
        // Create default config
        FEnvironmentalVFXConfig DefaultConfig;
        DefaultConfig.VFXType = VFXType;
        DefaultConfig.NiagaraVFXType = EVFXType::Environmental;
        VFXConfigs.Add(VFXType, DefaultConfig);
        Config = &VFXConfigs[VFXType];
    }
    
    // Apply intensity and global multipliers
    float FinalIntensity = Intensity * Config->IntensityMultiplier * GlobalIntensityMultiplier;
    
    // Get spawn location
    FVector SpawnLocation = GetRandomSpawnLocation(*Config, Location);
    
    // Check if we have too many active VFX of this type
    TArray<TWeakObjectPtr<UNiagaraComponent>>* ActiveVFXArray = ActiveVFX.Find(VFXType);
    if (ActiveVFXArray && ActiveVFXArray->Num() >= Config->MaxSimultaneousVFX)
    {
        // Remove oldest VFX
        if (ActiveVFXArray->Num() > 0 && (*ActiveVFXArray)[0].IsValid())
        {
            (*ActiveVFXArray)[0]->DestroyComponent();
            ActiveVFXArray->RemoveAt(0);
        }
    }
    
    // Spawn VFX through VFX Manager
    if (VFXManager)
    {
        UNiagaraComponent* VFXComponent = VFXManager->SpawnVFX(Config->NiagaraVFXType, SpawnLocation, FRotator::ZeroRotator);
        if (VFXComponent)
        {
            // Configure VFX parameters
            VFXComponent->SetFloatParameter(TEXT("Intensity"), FinalIntensity);
            VFXComponent->SetFloatParameter(TEXT("LifetimeMultiplier"), Config->LifetimeMultiplier);
            VFXComponent->SetVectorParameter(TEXT("Scale"), Config->ScaleRange);
            VFXComponent->SetFloatParameter(TEXT("WindSpeed"), CurrentWindSpeed);
            
            // Add to active VFX tracking
            if (!ActiveVFXArray)
            {
                ActiveVFX.Add(VFXType, TArray<TWeakObjectPtr<UNiagaraComponent>>());
                ActiveVFXArray = &ActiveVFX[VFXType];
            }
            ActiveVFXArray->Add(VFXComponent);
            
            UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Triggered VFX type %d at location %s with intensity %f"), 
                   (int32)VFXType, *SpawnLocation.ToString(), FinalIntensity);
        }
    }
    
    // Broadcast event
    OnEnvironmentalVFXTriggered.Broadcast(VFXType, SpawnLocation, FinalIntensity, CurrentWeatherState);
}

void UEnvironmentalVFXController::TriggerVFXAtLocation(EEnvironmentalVFXType VFXType, FVector Location, float Intensity)
{
    TriggerEnvironmentalVFX(VFXType, Location, Intensity);
}

void UEnvironmentalVFXController::TriggerRandomVFX(float Intensity)
{
    if (EnabledVFXTypes.Num() == 0)
    {
        return;
    }
    
    // Select random VFX type from enabled types
    int32 RandomIndex = FMath::RandRange(0, EnabledVFXTypes.Num() - 1);
    EEnvironmentalVFXType RandomVFXType = EnabledVFXTypes[RandomIndex];
    
    // Generate random location around owner
    FVector RandomLocation = FVector::ZeroVector;
    if (GetOwner())
    {
        RandomLocation = GetOwner()->GetActorLocation() + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-100.0f, 100.0f)
        );
    }
    
    TriggerEnvironmentalVFX(RandomVFXType, RandomLocation, Intensity);
}

void UEnvironmentalVFXController::TriggerTreeSway(float WindIntensity)
{
    if (!CanTriggerVFX(EEnvironmentalVFXType::TreeSway))
    {
        return;
    }
    
    // Find nearby static mesh components that could be trees
    if (GetOwner())
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
        
        for (AActor* Actor : NearbyActors)
        {
            if (FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation()) < 1000.0f)
            {
                UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
                if (MeshComp && MeshComp->GetStaticMesh())
                {
                    // Check if this looks like a tree (basic heuristic)
                    FVector MeshBounds = MeshComp->GetStaticMesh()->GetBounds().BoxExtent;
                    if (MeshBounds.Z > MeshBounds.X && MeshBounds.Z > MeshBounds.Y) // Tall object
                    {
                        TriggerEnvironmentalVFX(EEnvironmentalVFXType::TreeSway, Actor->GetActorLocation(), WindIntensity);
                    }
                }
            }
        }
    }
}

void UEnvironmentalVFXController::TriggerLeafFall(int32 LeafCount)
{
    if (!CanTriggerVFX(EEnvironmentalVFXType::LeafFall))
    {
        return;
    }
    
    for (int32 i = 0; i < LeafCount; ++i)
    {
        FVector LeafLocation = FVector::ZeroVector;
        if (GetOwner())
        {
            LeafLocation = GetOwner()->GetActorLocation() + FVector(
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(100.0f, 300.0f)
            );
        }
        
        TriggerEnvironmentalVFX(EEnvironmentalVFXType::LeafFall, LeafLocation, 1.0f);
    }
}

void UEnvironmentalVFXController::OnVegetationImpact(FVector ImpactLocation, float ImpactForce)
{
    // Trigger appropriate vegetation effects based on impact force
    if (ImpactForce > 100.0f)
    {
        TriggerEnvironmentalVFX(EEnvironmentalVFXType::BranchBreak, ImpactLocation, ImpactForce / 100.0f);
        TriggerLeafFall(FMath::RandRange(3, 8));
    }
    else if (ImpactForce > 50.0f)
    {
        TriggerEnvironmentalVFX(EEnvironmentalVFXType::GrassRustle, ImpactLocation, ImpactForce / 50.0f);
    }
    
    // Always trigger some dust
    TriggerGroundDust(ImpactLocation, ImpactForce / 200.0f);
}

void UEnvironmentalVFXController::TriggerWaterRipples(FVector WaterSurfaceLocation, float RippleIntensity)
{
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::WaterRipples, WaterSurfaceLocation, RippleIntensity);
}

void UEnvironmentalVFXController::TriggerWaterSplash(FVector SplashLocation, float SplashSize)
{
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::WaterSplash, SplashLocation, SplashSize);
}

void UEnvironmentalVFXController::StartRainEffect(float RainIntensity)
{
    if (bRainActive && FMath::IsNearlyEqual(CurrentRainIntensity, RainIntensity, 0.1f))
    {
        return; // Rain already active with similar intensity
    }
    
    // Stop existing rain first
    StopRainEffect();
    
    bRainActive = true;
    CurrentRainIntensity = RainIntensity;
    
    // Spawn rain VFX at multiple locations around the player
    if (GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            
            // Create rain grid around player
            for (int32 X = -2; X <= 2; ++X)
            {
                for (int32 Y = -2; Y <= 2; ++Y)
                {
                    FVector RainLocation = PlayerLocation + FVector(X * 1000.0f, Y * 1000.0f, 500.0f);
                    
                    if (VFXManager)
                    {
                        UNiagaraComponent* RainVFX = VFXManager->SpawnVFX(EVFXType::Weather, RainLocation, FRotator::ZeroRotator);
                        if (RainVFX)
                        {
                            RainVFX->SetFloatParameter(TEXT("RainIntensity"), RainIntensity);
                            RainVFX->SetFloatParameter(TEXT("WindSpeed"), CurrentWindSpeed);
                            ActiveRainVFX.Add(RainVFX);
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Started rain effect with intensity %f"), RainIntensity);
}

void UEnvironmentalVFXController::StopRainEffect()
{
    if (!bRainActive)
    {
        return;
    }
    
    bRainActive = false;
    CurrentRainIntensity = 0.0f;
    
    // Clean up rain VFX
    for (TWeakObjectPtr<UNiagaraComponent>& RainComponent : ActiveRainVFX)
    {
        if (RainComponent.IsValid())
        {
            RainComponent->DestroyComponent();
        }
    }
    ActiveRainVFX.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Stopped rain effect"));
}

void UEnvironmentalVFXController::TriggerLightningStrike(FVector StrikeLocation)
{
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::Lightning, StrikeLocation, 2.0f);
    
    // Also trigger some ground effects
    TriggerGroundDust(StrikeLocation, 1.5f);
    TriggerRockCrumble(StrikeLocation, 0.8f);
}

void UEnvironmentalVFXController::TriggerGroundDust(FVector Location, float DustIntensity)
{
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::Dust, Location, DustIntensity);
}

void UEnvironmentalVFXController::TriggerRockCrumble(FVector Location, float CrumbleSize)
{
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::RockCrumble, Location, CrumbleSize);
}

void UEnvironmentalVFXController::OnPlayerEnterArea()
{
    bPlayerInArea = true;
    
    // Trigger some ambient effects when player enters
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::Fireflies, FVector::ZeroVector, 0.5f);
    TriggerEnvironmentalVFX(EEnvironmentalVFXType::Insects, FVector::ZeroVector, 0.3f);
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Player entered area"));
}

void UEnvironmentalVFXController::OnPlayerExitArea()
{
    bPlayerInArea = false;
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Player exited area"));
}

void UEnvironmentalVFXController::OnPlayerMovement(FVector PlayerLocation, float MovementSpeed)
{
    LastPlayerLocation = PlayerLocation;
    LastPlayerMovementSpeed = MovementSpeed;
    
    // Trigger movement-based effects
    if (MovementSpeed > 300.0f) // Running
    {
        TriggerGroundDust(PlayerLocation, MovementSpeed / 600.0f);
    }
}

void UEnvironmentalVFXController::LoadVFXConfig(FName ConfigRowName)
{
    if (!EnvironmentalVFXConfigTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentalVFXController: No config table set"));
        return;
    }
    
    FEnvironmentalVFXConfig* Config = EnvironmentalVFXConfigTable->FindRow<FEnvironmentalVFXConfig>(ConfigRowName, TEXT("LoadVFXConfig"));
    if (Config)
    {
        VFXConfigs.Add(Config->VFXType, *Config);
        UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Loaded config for %s"), *ConfigRowName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentalVFXController: Could not find config row %s"), *ConfigRowName.ToString());
    }
}

void UEnvironmentalVFXController::EnableVFXType(EEnvironmentalVFXType VFXType, bool bEnable)
{
    if (bEnable)
    {
        EnabledVFXTypes.AddUnique(VFXType);
        if (!VFXTimers.Contains(VFXType))
        {
            VFXTimers.Add(VFXType, 0.0f);
        }
        if (!ActiveVFX.Contains(VFXType))
        {
            ActiveVFX.Add(VFXType, TArray<TWeakObjectPtr<UNiagaraComponent>>());
        }
    }
    else
    {
        EnabledVFXTypes.Remove(VFXType);
        
        // Clean up active VFX of this type
        TArray<TWeakObjectPtr<UNiagaraComponent>>* ActiveVFXArray = ActiveVFX.Find(VFXType);
        if (ActiveVFXArray)
        {
            for (TWeakObjectPtr<UNiagaraComponent>& VFXComponent : *ActiveVFXArray)
            {
                if (VFXComponent.IsValid())
                {
                    VFXComponent->DestroyComponent();
                }
            }
            ActiveVFXArray->Empty();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: %s VFX type %d"), 
           bEnable ? TEXT("Enabled") : TEXT("Disabled"), (int32)VFXType);
}

void UEnvironmentalVFXController::SetVFXIntensityMultiplier(float Multiplier)
{
    GlobalIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalVFXController: Set global intensity multiplier to %f"), GlobalIntensityMultiplier);
}

void UEnvironmentalVFXController::UpdateWeatherVFX(float DeltaTime)
{
    // Update weather-based effects based on current weather state
    switch (CurrentWeatherState)
    {
        case EWeatherState::Windy:
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime) // 10% chance per second
            {
                TriggerEnvironmentalVFX(EEnvironmentalVFXType::WindGust, FVector::ZeroVector, CurrentWindSpeed / 10.0f);
            }
            break;
            
        case EWeatherState::Foggy:
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f * DeltaTime) // 5% chance per second
            {
                TriggerEnvironmentalVFX(EEnvironmentalVFXType::FogRoll, FVector::ZeroVector, 1.0f);
            }
            break;
            
        case EWeatherState::Clear:
            if (FMath::RandRange(0.0f, 1.0f) < 0.02f * DeltaTime) // 2% chance per second
            {
                TriggerEnvironmentalVFX(EEnvironmentalVFXType::SunRays, FVector::ZeroVector, 0.8f);
            }
            break;
            
        default:
            break;
    }
}

void UEnvironmentalVFXController::UpdatePlayerProximityVFX(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
    
    bool bPlayerWasInArea = bPlayerInArea;
    bPlayerInArea = DistanceToPlayer <= PlayerDetectionRadius;
    
    // Handle player entering/exiting area
    if (bPlayerInArea && !bPlayerWasInArea)
    {
        OnPlayerEnterArea();
    }
    else if (!bPlayerInArea && bPlayerWasInArea)
    {
        OnPlayerExitArea();
    }
    
    // Update player movement tracking
    if (bPlayerInArea)
    {
        FVector PlayerVelocity = (PlayerLocation - LastPlayerLocation) / DeltaTime;
        float MovementSpeed = PlayerVelocity.Size();
        OnPlayerMovement(PlayerLocation, MovementSpeed);
    }
}

void UEnvironmentalVFXController::UpdateAutoTriggerVFX(float DeltaTime)
{
    // Update timers and trigger auto VFX
    for (auto& TimerPair : VFXTimers)
    {
        EEnvironmentalVFXType VFXType = TimerPair.Key;
        float& Timer = TimerPair.Value;
        
        Timer += DeltaTime;
        
        FEnvironmentalVFXConfig* Config = VFXConfigs.Find(VFXType);
        if (Config && Config->bAutoTrigger && Timer >= Config->TriggerInterval)
        {
            if (FMath::RandRange(0.0f, 1.0f) <= Config->SpawnChance)
            {
                TriggerEnvironmentalVFX(VFXType, FVector::ZeroVector, 1.0f);
            }
            Timer = 0.0f;
        }
    }
}

void UEnvironmentalVFXController::CleanupExpiredVFX()
{
    // Clean up invalid VFX components
    for (auto& VFXPair : ActiveVFX)
    {
        TArray<TWeakObjectPtr<UNiagaraComponent>>& VFXArray = VFXPair.Value;
        for (int32 i = VFXArray.Num() - 1; i >= 0; --i)
        {
            if (!VFXArray[i].IsValid())
            {
                VFXArray.RemoveAt(i);
            }
        }
    }
    
    // Clean up rain VFX
    for (int32 i = ActiveRainVFX.Num() - 1; i >= 0; --i)
    {
        if (!ActiveRainVFX[i].IsValid())
        {
            ActiveRainVFX.RemoveAt(i);
        }
    }
}

bool UEnvironmentalVFXController::CanTriggerVFX(EEnvironmentalVFXType VFXType) const
{
    // Check if VFX type is enabled
    if (!EnabledVFXTypes.Contains(VFXType))
    {
        return false;
    }
    
    // Check weather state requirements
    if (!IsWeatherStateValid(VFXType))
    {
        return false;
    }
    
    // Check wind speed requirements
    const FEnvironmentalVFXConfig* Config = VFXConfigs.Find(VFXType);
    if (Config)
    {
        if (CurrentWindSpeed < Config->MinWindSpeed || CurrentWindSpeed > Config->MaxWindSpeed)
        {
            return false;
        }
    }
    
    return true;
}

bool UEnvironmentalVFXController::IsWeatherStateValid(EEnvironmentalVFXType VFXType) const
{
    const FEnvironmentalVFXConfig* Config = VFXConfigs.Find(VFXType);
    if (!Config || Config->RequiredWeatherStates.Num() == 0)
    {
        return true; // No weather requirements
    }
    
    return Config->RequiredWeatherStates.Contains(CurrentWeatherState);
}

FVector UEnvironmentalVFXController::GetRandomSpawnLocation(const FEnvironmentalVFXConfig& Config, FVector BaseLocation) const
{
    FVector SpawnLocation = BaseLocation + Config.SpawnOffset;
    
    if (Config.bRandomizeSpawnLocation)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Config.RandomSpawnRadius, Config.RandomSpawnRadius),
            FMath::RandRange(-Config.RandomSpawnRadius, Config.RandomSpawnRadius),
            FMath::RandRange(-Config.RandomSpawnRadius * 0.5f, Config.RandomSpawnRadius * 0.5f)
        );
        SpawnLocation += RandomOffset;
    }
    
    return SpawnLocation;
}