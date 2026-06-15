#include "EnvArt_FallenLogAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Components/PrimitiveComponent.h"

AEnvArt_FallenLogAsset::AEnvArt_FallenLogAsset()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create log mesh component
    LogMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LogMeshComponent"));
    RootComponent = LogMeshComponent;
    
    // Set default collision
    LogMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LogMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    LogMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    LogMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    
    // Initialize default properties
    LogProperties.LogLength = FMath::RandRange(400.0f, 800.0f);
    LogProperties.LogDiameter = FMath::RandRange(60.0f, 120.0f);
    LogProperties.DecayState = EEnvArt_LogDecayState::Weathered;
    LogProperties.bHasMoss = FMath::RandBool();
    LogProperties.bHasLichen = FMath::RandBool();
    LogProperties.MossIntensity = FMath::RandRange(0.3f, 0.8f);
    LogProperties.bCanHidePlayer = (LogProperties.LogDiameter > 80.0f);
    LogProperties.bCanClimbOver = true;
    
    WeatheringTimer = 0.0f;
}

void AEnvArt_FallenLogAsset::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial material based on decay state
    UpdateLogMaterial();
    SetupCollisionForDecayState();
    
    // Scale the log based on properties
    FVector LogScale = FVector(
        LogProperties.LogLength / 100.0f,
        LogProperties.LogDiameter / 100.0f,
        LogProperties.LogDiameter / 100.0f
    );
    SetActorScale3D(LogScale);
}

void AEnvArt_FallenLogAsset::SetDecayState(EEnvArt_LogDecayState NewDecayState)
{
    LogProperties.DecayState = NewDecayState;
    UpdateLogMaterial();
    SetupCollisionForDecayState();
    
    // Adjust moss/lichen based on decay state
    switch (NewDecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
            LogProperties.bHasMoss = false;
            LogProperties.bHasLichen = false;
            LogProperties.MossIntensity = 0.0f;
            break;
            
        case EEnvArt_LogDecayState::Weathered:
            LogProperties.bHasMoss = FMath::RandBool();
            LogProperties.MossIntensity = FMath::RandRange(0.1f, 0.4f);
            break;
            
        case EEnvArt_LogDecayState::Mossy:
            LogProperties.bHasMoss = true;
            LogProperties.bHasLichen = true;
            LogProperties.MossIntensity = FMath::RandRange(0.5f, 0.9f);
            break;
            
        case EEnvArt_LogDecayState::Rotting:
            LogProperties.bHasMoss = true;
            LogProperties.bHasLichen = true;
            LogProperties.MossIntensity = FMath::RandRange(0.7f, 1.0f);
            LogProperties.bCanClimbOver = false;
            break;
            
        case EEnvArt_LogDecayState::Hollow:
            LogProperties.bHasMoss = true;
            LogProperties.bHasLichen = true;
            LogProperties.MossIntensity = 1.0f;
            LogProperties.bCanHidePlayer = true;
            LogProperties.bCanClimbOver = false;
            break;
    }
}

void AEnvArt_FallenLogAsset::SetMossIntensity(float NewMossIntensity)
{
    LogProperties.MossIntensity = FMath::Clamp(NewMossIntensity, 0.0f, 1.0f);
    UpdateLogMaterial();
}

void AEnvArt_FallenLogAsset::RandomizeLogProperties()
{
    LogProperties.LogLength = FMath::RandRange(300.0f, 1000.0f);
    LogProperties.LogDiameter = FMath::RandRange(50.0f, 150.0f);
    
    // Random decay state weighted toward weathered/mossy
    float DecayRoll = FMath::RandRange(0.0f, 1.0f);
    if (DecayRoll < 0.1f)
        LogProperties.DecayState = EEnvArt_LogDecayState::Fresh;
    else if (DecayRoll < 0.4f)
        LogProperties.DecayState = EEnvArt_LogDecayState::Weathered;
    else if (DecayRoll < 0.7f)
        LogProperties.DecayState = EEnvArt_LogDecayState::Mossy;
    else if (DecayRoll < 0.9f)
        LogProperties.DecayState = EEnvArt_LogDecayState::Rotting;
    else
        LogProperties.DecayState = EEnvArt_LogDecayState::Hollow;
    
    SetDecayState(LogProperties.DecayState);
    
    // Update scale
    FVector LogScale = FVector(
        LogProperties.LogLength / 100.0f,
        LogProperties.LogDiameter / 100.0f,
        LogProperties.LogDiameter / 100.0f
    );
    SetActorScale3D(LogScale);
}

bool AEnvArt_FallenLogAsset::CanPlayerHideBehind() const
{
    return LogProperties.bCanHidePlayer && LogProperties.LogDiameter > 80.0f;
}

FVector AEnvArt_FallenLogAsset::GetClimbOverPoint() const
{
    if (!LogProperties.bCanClimbOver)
        return FVector::ZeroVector;
    
    FVector LogCenter = GetActorLocation();
    FVector LogForward = GetActorForwardVector();
    
    // Return point on top of log at center
    return LogCenter + FVector(0, 0, LogProperties.LogDiameter * 0.5f);
}

void AEnvArt_FallenLogAsset::ApplyEnvironmentalWeathering(float DeltaTime)
{
    WeatheringTimer += DeltaTime;
    
    // Slowly increase moss over time (very slow process)
    if (LogProperties.bHasMoss && WeatheringTimer > 60.0f) // Every minute
    {
        LogProperties.MossIntensity = FMath::Min(1.0f, LogProperties.MossIntensity + 0.01f);
        WeatheringTimer = 0.0f;
        UpdateLogMaterial();
    }
}

void AEnvArt_FallenLogAsset::UpdateLogMaterial()
{
    if (!LogMeshComponent)
        return;
    
    UMaterialInterface* TargetMaterial = nullptr;
    
    switch (LogProperties.DecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
            TargetMaterial = FreshLogMaterial;
            break;
        case EEnvArt_LogDecayState::Weathered:
            TargetMaterial = WeatheredLogMaterial;
            break;
        case EEnvArt_LogDecayState::Mossy:
            TargetMaterial = MossyLogMaterial;
            break;
        case EEnvArt_LogDecayState::Rotting:
            TargetMaterial = RottenLogMaterial;
            break;
        case EEnvArt_LogDecayState::Hollow:
            TargetMaterial = HollowLogMaterial;
            break;
    }
    
    if (TargetMaterial)
    {
        LogMeshComponent->SetMaterial(0, TargetMaterial);
    }
}

void AEnvArt_FallenLogAsset::SetupCollisionForDecayState()
{
    if (!LogMeshComponent)
        return;
    
    switch (LogProperties.DecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
        case EEnvArt_LogDecayState::Weathered:
        case EEnvArt_LogDecayState::Mossy:
            // Solid collision
            LogMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
            
        case EEnvArt_LogDecayState::Rotting:
            // Softer collision - player can push through with effort
            LogMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
            break;
            
        case EEnvArt_LogDecayState::Hollow:
            // Player can walk through hollow section
            LogMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
            break;
    }
}