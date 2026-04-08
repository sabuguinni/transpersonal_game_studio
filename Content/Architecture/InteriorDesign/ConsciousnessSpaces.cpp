#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "ConsciousnessSpaces.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessSpace : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessSpace();

protected:
    virtual void BeginPlay() override;

    // Core architectural components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* BaseStructure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* SacredGeometryElements;

    // Consciousness-responsive lighting
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    ULightComponent* ConsciousnessAmbientLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    TArray<ULightComponent*> ChakraAlignmentLights;

    // Energy flow visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy")
    UParticleSystemComponent* EnergyFlowParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy")
    UParticleSystemComponent* ConsciousnessResonanceField;

    // Interior space configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FVector MeditationZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    float MeditationZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    int32 MaxOccupancy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TArray<FVector> SacredGeometryPoints;

public:
    virtual void Tick(float DeltaTime) override;

    // Consciousness-responsive functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessLighting(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ActivateEnergyFlow(bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ConfigureMeditationSpace(int32 ParticipantCount);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetSacredGeometryPattern(const FString& PatternType);

    // Procedural interior generation
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void GenerateInteriorLayout(const FString& SpaceType, float ConsciousnessResonance);

protected:
    // Interior design helper functions
    void CreateMeditationSeatingArrangement();
    void SetupConsciousnessAmplificationZones();
    void ConfigureEnergyCirculationPaths();
    void PlaceSacredArtifacts();

    // Consciousness integration
    void UpdateSpaceResonance(float PlayerConsciousnessLevel);
    void AdjustAcousticProperties(float MeditationDepth);
    void ModulateEnergyFieldIntensity(int32 GroupConsciousnessLevel);

private:
    // Internal state tracking
    float CurrentConsciousnessResonance;
    int32 CurrentOccupantCount;
    FString ActiveGeometryPattern;
    bool bEnergyFlowActive;

    // Sacred geometry calculations
    TArray<FVector> CalculateGoldenRatioSpiral(FVector Center, float Radius);
    TArray<FVector> GenerateFlowerOfLifePattern(FVector Center, float Scale);
    TArray<FVector> CreateMandalaPoints(FVector Center, int32 Layers, float BaseRadius);
};

// Implementation
AConsciousnessSpace::AConsciousnessSpace()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize base structure
    BaseStructure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseStructure"));
    RootComponent = BaseStructure;

    SacredGeometryElements = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SacredGeometry"));
    SacredGeometryElements->SetupAttachment(RootComponent);

    // Initialize lighting system
    ConsciousnessAmbientLight = CreateDefaultSubobject<ULightComponent>(TEXT("ConsciousnessLight"));
    ConsciousnessAmbientLight->SetupAttachment(RootComponent);

    // Initialize energy systems
    EnergyFlowParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EnergyFlow"));
    EnergyFlowParticles->SetupAttachment(RootComponent);

    ConsciousnessResonanceField = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ResonanceField"));
    ConsciousnessResonanceField->SetupAttachment(RootComponent);

    // Default values
    MeditationZoneRadius = 750.0f; // 7.5 meters
    MaxOccupancy = 12;
    CurrentConsciousnessResonance = 0.5f;
    bEnergyFlowActive = false;
}

void AConsciousnessSpace::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize sacred geometry pattern
    SetSacredGeometryPattern("FlowerOfLife");
    
    // Setup initial meditation space
    CreateMeditationSeatingArrangement();
    
    // Configure consciousness amplification zones
    SetupConsciousnessAmplificationZones();
}

void AConsciousnessSpace::UpdateConsciousnessLighting(float ConsciousnessLevel)
{
    if (ConsciousnessAmbientLight)
    {
        // Adjust light intensity based on consciousness level
        float LightIntensity = FMath::Lerp(0.3f, 2.0f, ConsciousnessLevel);
        ConsciousnessAmbientLight->SetIntensity(LightIntensity);

        // Shift color temperature with consciousness depth
        FLinearColor LightColor = FLinearColor::LerpUsingHSV(
            FLinearColor(1.0f, 0.8f, 0.6f), // Warm earthly
            FLinearColor(0.6f, 0.8f, 1.0f), // Cool cosmic
            ConsciousnessLevel
        );
        ConsciousnessAmbientLight->SetLightColor(LightColor);
    }

    // Update chakra alignment lights
    for (int32 i = 0; i < ChakraAlignmentLights.Num(); i++)
    {
        if (ChakraAlignmentLights[i])
        {
            float ChakraIntensity = FMath::Sin((ConsciousnessLevel + i * 0.1f) * PI) * 0.5f + 0.5f;
            ChakraAlignmentLights[i]->SetIntensity(ChakraIntensity);
        }
    }
}

void AConsciousnessSpace::GenerateInteriorLayout(const FString& SpaceType, float ConsciousnessResonance)
{
    if (SpaceType == "MeditationSanctuary")
    {
        // Create concentric seating circles
        CreateMeditationSeatingArrangement();
        
        // Place consciousness amplification crystals
        SetupConsciousnessAmplificationZones();
        
        // Configure energy circulation
        ConfigureEnergyCirculationPaths();
    }
    else if (SpaceType == "WisdomLibrary")
    {
        // Generate knowledge storage areas
        // Implement spiral access paths
        // Create study alcoves
    }
    else if (SpaceType == "TransformationChamber")
    {
        // Setup transformation ritual space
        // Configure consciousness evolution tools
        // Place integration support elements
    }
}

TArray<FVector> AConsciousnessSpace::CalculateGoldenRatioSpiral(FVector Center, float Radius)
{
    TArray<FVector> SpiralPoints;
    const float GoldenRatio = 1.618033988749f;
    const int32 PointCount = 21; // Sacred number
    
    for (int32 i = 0; i < PointCount; i++)
    {
        float Angle = i * 2.0f * PI / GoldenRatio;
        float Distance = Radius * FMath::Sqrt(i) / FMath::Sqrt(PointCount);
        
        FVector Point = Center + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        SpiralPoints.Add(Point);
    }
    
    return SpiralPoints;
}