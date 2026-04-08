#include "SacredArchitecture.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ASacredArchitecture::ASacredArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh
    MainStructure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructure"));
    MainStructure->SetupAttachment(RootComponent);

    // Create detail elements mesh
    DetailElements = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DetailElements"));
    DetailElements->SetupAttachment(RootComponent);

    // Initialize default values
    ArchitecturalStyle = EArchitecturalStyle::Temple;
    bRespondsToConsciousness = true;
    BaseResonanceFrequency = 432.0f; // Sacred frequency
    SacredProportions = FVector(1.618f, 1.0f, 0.618f); // Golden ratio proportions
    
    CurrentConsciousnessLevel = EConsciousnessLevel::Ego;
    bIsTransforming = false;
    TransformationProgress = 0.0f;
    
    CurrentTransformationTime = 0.0f;
    TotalTransformationTime = 3.0f;

    // Setup default transformations for each consciousness level
    ConsciousnessTransformations.SetNum(6);
    
    // Ego Level
    ConsciousnessTransformations[0].TriggerLevel = EConsciousnessLevel::Ego;
    ConsciousnessTransformations[0].ScaleMultiplier = FVector(1.0f);
    ConsciousnessTransformations[0].EmissiveColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    
    // Personal Unconscious
    ConsciousnessTransformations[1].TriggerLevel = EConsciousnessLevel::Personal;
    ConsciousnessTransformations[1].ScaleMultiplier = FVector(1.2f);
    ConsciousnessTransformations[1].EmissiveColor = FLinearColor(0.6f, 0.8f, 0.9f, 1.0f);
    
    // Collective Unconscious
    ConsciousnessTransformations[2].TriggerLevel = EConsciousnessLevel::Collective;
    ConsciousnessTransformations[2].ScaleMultiplier = FVector(1.5f);
    ConsciousnessTransformations[2].EmissiveColor = FLinearColor(0.9f, 0.7f, 0.9f, 1.0f);
    
    // Archetypal
    ConsciousnessTransformations[3].TriggerLevel = EConsciousnessLevel::Archetypal;
    ConsciousnessTransformations[3].ScaleMultiplier = FVector(2.0f);
    ConsciousnessTransformations[3].EmissiveColor = FLinearColor(1.0f, 0.9f, 0.6f, 1.0f);
    
    // Cosmic Consciousness
    ConsciousnessTransformations[4].TriggerLevel = EConsciousnessLevel::Cosmic;
    ConsciousnessTransformations[4].ScaleMultiplier = FVector(3.0f);
    ConsciousnessTransformations[4].EmissiveColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    
    // Unity Consciousness
    ConsciousnessTransformations[5].TriggerLevel = EConsciousnessLevel::Unity;
    ConsciousnessTransformations[5].ScaleMultiplier = FVector(5.0f);
    ConsciousnessTransformations[5].EmissiveColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void ASacredArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    // Store initial transformation values
    InitialScale = GetActorScale3D();
    InitialRotation = GetActorRotation();
    InitialEmissiveColor = FLinearColor::White;
    InitialOpacity = 1.0f;
    
    // Apply sacred geometry proportions
    ApplyGoldenRatioProportions();
    
    // Set initial consciousness level transformation
    SetConsciousnessLevel(CurrentConsciousnessLevel);
}

void ASacredArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsTransforming)
    {
        UpdateArchitecturalTransformation(DeltaTime);
    }
    
    // Update sacred resonance based on time
    if (bRespondsToConsciousness)
    {
        float ResonancePhase = GetWorld()->GetTimeSeconds() * BaseResonanceFrequency * 0.01f;
        float ResonanceIntensity = FMath::Sin(ResonancePhase) * 0.1f + 1.0f;
        
        // Apply subtle pulsing to emissive materials
        if (MainStructure && MainStructure->GetMaterial(0))
        {
            UMaterialInstanceDynamic* DynamicMaterial = MainStructure->CreateAndSetMaterialInstanceDynamic(0);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("ResonanceIntensity"), ResonanceIntensity);
            }
        }
    }
}

void ASacredArchitecture::SetConsciousnessLevel(EConsciousnessLevel NewLevel)
{
    if (NewLevel == CurrentConsciousnessLevel)
    {
        return;
    }
    
    EConsciousnessLevel OldLevel = CurrentConsciousnessLevel;
    CurrentConsciousnessLevel = NewLevel;
    
    // Find transformation for this level
    FArchitecturalTransformation* Transformation = GetTransformationForLevel(NewLevel);
    if (Transformation)
    {
        // Set target values
        TargetScale = InitialScale * Transformation->ScaleMultiplier;
        TargetRotation = InitialRotation + Transformation->RotationOffset;
        TargetEmissiveColor = Transformation->EmissiveColor;
        TargetOpacity = Transformation->MaterialOpacity;
        
        // Start transformation
        bIsTransforming = true;
        CurrentTransformationTime = 0.0f;
        TotalTransformationTime = Transformation->TransformationDuration;
        TransformationProgress = 0.0f;
        
        // Store current values as starting point
        InitialScale = GetActorScale3D();
        InitialRotation = GetActorRotation();
    }
    
    // Trigger blueprint event
    OnConsciousnessLevelChanged(OldLevel, NewLevel);
}

void ASacredArchitecture::TriggerSacredResonance()
{
    // Create a resonance wave effect
    float ResonanceAmplitude = 1.5f;
    float ResonanceDuration = 2.0f;
    
    // Apply temporary scale pulse
    FVector CurrentScale = GetActorScale3D();
    FVector ResonanceScale = CurrentScale * ResonanceAmplitude;
    
    // This could be enhanced with a timeline component for smooth animation
    SetActorScale3D(ResonanceScale);
    
    // Reset scale after duration (simplified - in production use timeline)
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this, CurrentScale]()
        {
            SetActorScale3D(CurrentScale);
        },
        ResonanceDuration,
        false
    );
}

void ASacredArchitecture::ApplyGoldenRatioProportions()
{
    if (MainStructure)
    {
        FVector CurrentScale = MainStructure->GetRelativeScale3D();
        FVector GoldenScale = CurrentScale;
        
        // Apply golden ratio to dimensions
        GoldenScale.X *= SacredProportions.X; // 1.618 (phi)
        GoldenScale.Y *= SacredProportions.Y; // 1.0
        GoldenScale.Z *= SacredProportions.Z; // 0.618 (1/phi)
        
        MainStructure->SetRelativeScale3D(GoldenScale);
    }
}

void ASacredArchitecture::UpdateArchitecturalTransformation(float DeltaTime)
{
    CurrentTransformationTime += DeltaTime;
    TransformationProgress = FMath::Clamp(CurrentTransformationTime / TotalTransformationTime, 0.0f, 1.0f);
    
    // Use smooth interpolation curve
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, TransformationProgress);
    
    // Interpolate scale
    FVector CurrentScale = FMath::Lerp(InitialScale, TargetScale, SmoothProgress);
    SetActorScale3D(CurrentScale);
    
    // Interpolate rotation
    FRotator CurrentRotation = FMath::Lerp(InitialRotation, TargetRotation, SmoothProgress);
    SetActorRotation(CurrentRotation);
    
    // Update material properties
    if (MainStructure && MainStructure->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = MainStructure->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            FLinearColor CurrentEmissive = FMath::Lerp(InitialEmissiveColor, TargetEmissiveColor, SmoothProgress);
            float CurrentOpacity = FMath::Lerp(InitialOpacity, TargetOpacity, SmoothProgress);
            
            DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), CurrentEmissive);
            DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), CurrentOpacity);
        }
    }
    
    // Check if transformation is complete
    if (TransformationProgress >= 1.0f)
    {
        bIsTransforming = false;
        TransformationProgress = 1.0f;
        OnTransformationComplete();
    }
}

void ASacredArchitecture::CalculateSacredGeometry()
{
    // Calculate positions based on sacred geometric principles
    float GoldenRatio = 1.618033988749f;
    float GoldenAngle = 137.507764f; // Golden angle in degrees
    
    // Apply sacred proportions to detail elements
    if (DetailElements)
    {
        int32 NumElements = 8; // Octagonal sacred pattern
        float Radius = 500.0f;
        
        for (int32 i = 0; i < NumElements; i++)
        {
            float Angle = (GoldenAngle * i) * PI / 180.0f;
            float CurrentRadius = Radius * FMath::Pow(GoldenRatio, i * 0.1f);
            
            FVector ElementPosition;
            ElementPosition.X = CurrentRadius * FMath::Cos(Angle);
            ElementPosition.Y = CurrentRadius * FMath::Sin(Angle);
            ElementPosition.Z = i * 50.0f; // Spiral upward
            
            // This would be used to position multiple detail elements
            // In a full implementation, you'd create multiple components or actors
        }
    }
}

FArchitecturalTransformation* ASacredArchitecture::GetTransformationForLevel(EConsciousnessLevel Level)
{
    for (FArchitecturalTransformation& Transformation : ConsciousnessTransformations)
    {
        if (Transformation.TriggerLevel == Level)
        {
            return &Transformation;
        }
    }
    return nullptr;
}