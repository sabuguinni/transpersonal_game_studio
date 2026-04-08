#include "ConsciousnessEnvironment.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

AConsciousnessEnvironment::AConsciousnessEnvironment()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create terrain mesh
    TerrainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainMesh"));
    TerrainMesh->SetupAttachment(RootComponent);

    // Create vegetation mesh
    VegetationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VegetationMesh"));
    VegetationMesh->SetupAttachment(RootComponent);

    // Create sacred geometry
    SacredGeometry = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SacredGeometry"));
    SacredGeometry->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentState = EConsciousnessState::Ordinary;
    CurrentBiome = EEnvironmentBiome::SacredForest;
    ConsciousnessLevel = 0.0f;
    SpiritualResonance = 0.0f;

    // Initialize transition settings
    TransitionSettings.TransitionDuration = 3.0f;
    TransitionSettings.bSmoothTransition = true;
    TransitionSettings.GeometryMorphSpeed = 1.0f;

    // Initialize geometry animation
    GeometryRotationSpeed = 30.0f;
    GeometryFloatAmplitude = 50.0f;
    GeometryFloatFrequency = 1.0f;
}

void AConsciousnessEnvironment::BeginPlay()
{
    Super::BeginPlay();

    // Store initial geometry transform
    if (SacredGeometry)
    {
        InitialGeometryLocation = SacredGeometry->GetRelativeLocation();
        InitialGeometryRotation = SacredGeometry->GetRelativeRotation();
    }

    // Find lighting actors in the world
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            SunLight = *ActorItr;
            break;
        }
    }

    if (!AmbientLight)
    {
        for (TActorIterator<ASkyLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AmbientLight = *ActorItr;
            break;
        }
    }

    // Initialize material parameters
    UpdateMaterialParameters();
    UpdateLighting();
    UpdateBiomeCharacteristics();
}

void AConsciousnessEnvironment::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update geometry animation
    UpdateGeometryAnimation(DeltaTime);

    // Process transitions
    if (bIsTransitioning)
    {
        ProcessTransition(DeltaTime);
    }

    // Apply consciousness effects
    ApplyConsciousnessEffects();

    // Update material parameters continuously
    UpdateMaterialParameters();
}

void AConsciousnessEnvironment::SetConsciousnessState(EConsciousnessState NewState)
{
    if (NewState != CurrentState)
    {
        TriggerEnvironmentTransition(NewState, CurrentBiome);
    }
}

void AConsciousnessEnvironment::SetEnvironmentBiome(EEnvironmentBiome NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        TriggerEnvironmentTransition(CurrentState, NewBiome);
    }
}

void AConsciousnessEnvironment::UpdateConsciousnessLevel(float NewLevel)
{
    ConsciousnessLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    
    // Trigger visual effects based on consciousness level
    if (ConsciousnessLevel > 0.8f)
    {
        SpawnLightParticles(GetActorLocation(), ConsciousnessLevel);
    }
    
    if (ConsciousnessLevel > 0.9f)
    {
        CreateEnergyWaves(GetActorLocation(), 1000.0f * ConsciousnessLevel);
    }
}

void AConsciousnessEnvironment::UpdateSpiritualResonance(float NewResonance)
{
    SpiritualResonance = FMath::Clamp(NewResonance, 0.0f, 1.0f);
    
    // Update geometry animation speed based on resonance
    GeometryRotationSpeed = 30.0f + (SpiritualResonance * 60.0f);
    GeometryFloatFrequency = 1.0f + (SpiritualResonance * 2.0f);
    
    // Manifest sacred symbols at high resonance
    if (SpiritualResonance > 0.7f)
    {
        ManifestSacredSymbols(GetActorLocation(), SpiritualResonance);
    }
}

void AConsciousnessEnvironment::TriggerEnvironmentTransition(EConsciousnessState TargetState, EEnvironmentBiome TargetBiome)
{
    if (bIsTransitioning)
    {
        return; // Already transitioning
    }

    // Store current values for interpolation
    PreviousAmbientColor = GetStateColor(CurrentState);
    TargetAmbientColor = GetStateColor(TargetState);
    PreviousLightIntensity = GetStateLightIntensity(CurrentState);
    TargetLightIntensity = GetStateLightIntensity(TargetState);

    // Start transition
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    CurrentState = TargetState;
    CurrentBiome = TargetBiome;

    UE_LOG(LogTemp, Warning, TEXT("Environment transition started to state %d, biome %d"), 
           (int32)TargetState, (int32)TargetBiome);
}

void AConsciousnessEnvironment::SpawnLightParticles(FVector Location, float Intensity)
{
    // Create light particle effects
    if (GetWorld())
    {
        FVector SpawnLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(100.0f, 300.0f)
        );

        // Spawn particle system (would be replaced with actual particle system in full implementation)
        UE_LOG(LogTemp, Log, TEXT("Spawning light particles at %s with intensity %f"), 
               *SpawnLocation.ToString(), Intensity);
    }
}

void AConsciousnessEnvironment::CreateEnergyWaves(FVector Center, float Radius)
{
    // Create expanding energy wave effects
    float WaveSpeed = 500.0f;
    float WaveLifetime = Radius / WaveSpeed;

    UE_LOG(LogTemp, Log, TEXT("Creating energy waves at %s with radius %f"), 
           *Center.ToString(), Radius);

    // Implementation would create expanding ring effects
}

void AConsciousnessEnvironment::ManifestSacredSymbols(FVector Location, float Scale)
{
    // Manifest sacred geometry symbols
    if (GeometryMeshes.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, GeometryMeshes.Num() - 1);
        UStaticMesh* SelectedMesh = GeometryMeshes[RandomIndex];

        if (SelectedMesh && SacredGeometry)
        {
            SacredGeometry->SetStaticMesh(SelectedMesh);
            SacredGeometry->SetWorldScale3D(FVector(Scale));
            
            UE_LOG(LogTemp, Log, TEXT("Manifesting sacred symbol at %s with scale %f"), 
                   *Location.ToString(), Scale);
        }
    }
}

void AConsciousnessEnvironment::GenerateTerrainForState(EConsciousnessState State)
{
    // Generate terrain based on consciousness state
    switch (State)
    {
        case EConsciousnessState::Ordinary:
            // Natural, earthly terrain
            break;
        case EConsciousnessState::Meditative:
            // Smooth, flowing terrain with gentle hills
            break;
        case EConsciousnessState::Transcendent:
            // Ethereal, floating terrain elements
            break;
        case EConsciousnessState::Unity:
            // Interconnected, mandala-like patterns
            break;
        case EConsciousnessState::Cosmic:
            // Abstract, cosmic geometry
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Generating terrain for consciousness state %d"), (int32)State);
}

void AConsciousnessEnvironment::PopulateVegetation(float Density, float ConsciousnessInfluence)
{
    // Populate vegetation based on consciousness influence
    if (VegetationMesh)
    {
        // Adjust vegetation properties based on consciousness
        FVector Scale = FVector(1.0f + ConsciousnessInfluence);
        VegetationMesh->SetWorldScale3D(Scale);

        UE_LOG(LogTemp, Log, TEXT("Populating vegetation with density %f and consciousness influence %f"), 
               Density, ConsciousnessInfluence);
    }
}

void AConsciousnessEnvironment::PlaceSacredGeometry(int32 Count, float MinDistance)
{
    // Place sacred geometry elements in the environment
    for (int32 i = 0; i < Count; ++i)
    {
        FVector RandomLocation = GetActorLocation() + FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(0.0f, 500.0f)
        );

        UE_LOG(LogTemp, Log, TEXT("Placing sacred geometry %d at %s"), i, *RandomLocation.ToString());
    }
}

void AConsciousnessEnvironment::UpdateMaterialParameters()
{
    if (EnvironmentParameters)
    {
        UMaterialParameterCollectionInstance* ParameterInstance = 
            GetWorld()->GetParameterCollectionInstance(EnvironmentParameters);

        if (ParameterInstance)
        {
            // Update consciousness level parameter
            ParameterInstance->SetScalarParameterValue(TEXT("ConsciousnessLevel"), ConsciousnessLevel);
            
            // Update spiritual resonance parameter
            ParameterInstance->SetScalarParameterValue(TEXT("SpiritualResonance"), SpiritualResonance);
            
            // Update state-specific parameters
            ParameterInstance->SetScalarParameterValue(TEXT("StateIndex"), (float)CurrentState);
            ParameterInstance->SetScalarParameterValue(TEXT("BiomeIndex"), (float)CurrentBiome);
            
            // Update transition progress
            ParameterInstance->SetScalarParameterValue(TEXT("TransitionProgress"), TransitionProgress);
        }
    }
}

void AConsciousnessEnvironment::UpdateLighting()
{
    FLinearColor CurrentColor = bIsTransitioning ? 
        FMath::Lerp(PreviousAmbientColor, TargetAmbientColor, TransitionProgress) :
        GetStateColor(CurrentState);

    float CurrentIntensity = bIsTransitioning ?
        FMath::Lerp(PreviousLightIntensity, TargetLightIntensity, TransitionProgress) :
        GetStateLightIntensity(CurrentState);

    // Update directional light
    if (SunLight)
    {
        SunLight->GetLightComponent()->SetLightColor(CurrentColor);
        SunLight->GetLightComponent()->SetIntensity(CurrentIntensity);
    }

    // Update ambient light
    if (AmbientLight)
    {
        AmbientLight->GetLightComponent()->SetLightColor(CurrentColor);
        AmbientLight->GetLightComponent()->SetIntensity(CurrentIntensity * 0.3f);
    }
}

void AConsciousnessEnvironment::UpdateGeometryAnimation(float DeltaTime)
{
    if (!SacredGeometry)
        return;

    GeometryAnimationTime += DeltaTime;

    // Rotation animation
    FRotator NewRotation = InitialGeometryRotation;
    NewRotation.Yaw += GeometryRotationSpeed * GeometryAnimationTime;
    NewRotation.Pitch += (GeometryRotationSpeed * 0.5f) * GeometryAnimationTime;
    SacredGeometry->SetRelativeRotation(NewRotation);

    // Floating animation
    FVector NewLocation = InitialGeometryLocation;
    NewLocation.Z += FMath::Sin(GeometryAnimationTime * GeometryFloatFrequency) * GeometryFloatAmplitude;
    SacredGeometry->SetRelativeLocation(NewLocation);

    // Scale pulsing based on consciousness level
    float ScalePulse = 1.0f + (ConsciousnessLevel * 0.2f * FMath::Sin(GeometryAnimationTime * 2.0f));
    SacredGeometry->SetWorldScale3D(FVector(ScalePulse));
}

void AConsciousnessEnvironment::ProcessTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionSettings.TransitionDuration;

    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        UE_LOG(LogTemp, Warning, TEXT("Environment transition completed"));
    }

    // Apply transition curve if available
    float CurvedProgress = TransitionProgress;
    if (TransitionSettings.TransitionCurve)
    {
        CurvedProgress = TransitionSettings.TransitionCurve->GetFloatValue(TransitionProgress);
    }

    // Update lighting with curved progress
    UpdateLighting();

    // Update biome characteristics
    UpdateBiomeCharacteristics();
}

void AConsciousnessEnvironment::ApplyConsciousnessEffects()
{
    // Apply visual effects based on current consciousness level
    if (ConsciousnessLevel > 0.5f)
    {
        // Increase environment luminosity
        float LuminosityBoost = (ConsciousnessLevel - 0.5f) * 2.0f;
        
        if (EnvironmentParameters)
        {
            UMaterialParameterCollectionInstance* ParameterInstance = 
                GetWorld()->GetParameterCollectionInstance(EnvironmentParameters);
            
            if (ParameterInstance)
            {
                ParameterInstance->SetScalarParameterValue(TEXT("LuminosityBoost"), LuminosityBoost);
            }
        }
    }

    // Apply spiritual resonance effects
    if (SpiritualResonance > 0.6f)
    {
        // Increase sacred geometry visibility and animation
        if (SacredGeometry)
        {
            float Visibility = SpiritualResonance;
            SacredGeometry->SetVisibility(Visibility > 0.3f);
        }
    }
}

void AConsciousnessEnvironment::UpdateBiomeCharacteristics()
{
    // Update environment based on current biome
    FVector BiomeScale = GetBiomeScale(CurrentBiome);
    UMaterialInterface* BiomeMaterial = GetBiomeMaterial(CurrentBiome);

    if (TerrainMesh && BiomeMaterial)
    {
        TerrainMesh->SetMaterial(0, BiomeMaterial);
        TerrainMesh->SetWorldScale3D(BiomeScale);
    }
}

FLinearColor AConsciousnessEnvironment::GetStateColor(EConsciousnessState State)
{
    switch (State)
    {
        case EConsciousnessState::Ordinary:
            return FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm daylight
        case EConsciousnessState::Meditative:
            return FLinearColor(0.7f, 0.8f, 1.0f, 1.0f); // Cool blue
        case EConsciousnessState::Transcendent:
            return FLinearColor(1.0f, 0.8f, 1.0f, 1.0f); // Soft purple
        case EConsciousnessState::Unity:
            return FLinearColor(1.0f, 1.0f, 0.8f, 1.0f); // Golden light
        case EConsciousnessState::Cosmic:
            return FLinearColor(0.9f, 0.9f, 1.0f, 1.0f); // Cosmic white
        default:
            return FLinearColor::White;
    }
}

float AConsciousnessEnvironment::GetStateLightIntensity(EConsciousnessState State)
{
    switch (State)
    {
        case EConsciousnessState::Ordinary:
            return 3.0f;
        case EConsciousnessState::Meditative:
            return 2.0f;
        case EConsciousnessState::Transcendent:
            return 4.0f;
        case EConsciousnessState::Unity:
            return 6.0f;
        case EConsciousnessState::Cosmic:
            return 8.0f;
        default:
            return 3.0f;
    }
}

FVector AConsciousnessEnvironment::GetBiomeScale(EEnvironmentBiome Biome)
{
    switch (Biome)
    {
        case EEnvironmentBiome::SacredForest:
            return FVector(1.0f, 1.0f, 1.2f);
        case EEnvironmentBiome::CrystalCaves:
            return FVector(0.8f, 0.8f, 0.6f);
        case EEnvironmentBiome::FloatingIslands:
            return FVector(1.5f, 1.5f, 2.0f);
        case EEnvironmentBiome::LightRealm:
            return FVector(2.0f, 2.0f, 3.0f);
        case EEnvironmentBiome::VoidSpace:
            return FVector(0.5f, 0.5f, 0.5f);
        case EEnvironmentBiome::MandalaGarden:
            return FVector(1.0f, 1.0f, 1.0f);
        default:
            return FVector::OneVector;
    }
}

UMaterialInterface* AConsciousnessEnvironment::GetBiomeMaterial(EEnvironmentBiome Biome)
{
    // Return appropriate material based on biome
    // In a full implementation, these would be loaded from content
    switch (Biome)
    {
        case EEnvironmentBiome::SacredForest:
            return TerrainMaterial; // Forest material
        case EEnvironmentBiome::CrystalCaves:
            return TerrainMaterial; // Crystal material
        case EEnvironmentBiome::FloatingIslands:
            return TerrainMaterial; // Ethereal material
        case EEnvironmentBiome::LightRealm:
            return TerrainMaterial; // Light material
        case EEnvironmentBiome::VoidSpace:
            return TerrainMaterial; // Void material
        case EEnvironmentBiome::MandalaGarden:
            return TerrainMaterial; // Mandala material
        default:
            return TerrainMaterial;
    }
}