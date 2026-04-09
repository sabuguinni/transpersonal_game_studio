#include "MetaHumanCharacterComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"

UMetaHumanCharacterComponent::UMetaHumanCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize MetaHuman settings
    CurrentBodyType = EMetaHumanBodyType::Medium;
    CurrentGender = EMetaHumanGender::Male;
    CurrentAgeRange = EMetaHumanAgeRange::Adult;
    
    bIsMetaHumanActive = false;
    bUseProceduralFacialAnimation = true;
    bUseLODSystem = true;
    
    FacialExpressionIntensity = 1.0f;
    EmotionalResponseMultiplier = 1.0f;
    
    InitializeMetaHumanPresets();
}

void UMetaHumanCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bIsMetaHumanActive)
    {
        InitializeMetaHuman();
    }
    
    UE_LOG(LogTemp, Log, TEXT("MetaHuman Character Component initialized"));
}

void UMetaHumanCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMetaHumanActive)
    {
        UpdateFacialExpressions(DeltaTime);
        UpdateLODSystem(DeltaTime);
    }
}

void UMetaHumanCharacterComponent::InitializeMetaHuman()
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Set up MetaHuman mesh based on current configuration
    ApplyMetaHumanConfiguration();
    
    // Initialize facial animation system
    if (bUseProceduralFacialAnimation)
    {
        SetupFacialAnimationSystem();
    }
    
    // Set up LOD system
    if (bUseLODSystem)
    {
        SetupLODSystem();
    }
    
    bIsMetaHumanActive = true;
    OnMetaHumanInitialized.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHuman initialized successfully"));
}

void UMetaHumanCharacterComponent::SetBodyType(EMetaHumanBodyType NewBodyType)
{
    if (CurrentBodyType != NewBodyType)
    {
        CurrentBodyType = NewBodyType;
        if (bIsMetaHumanActive)
        {
            ApplyBodyTypeChanges();
        }
        OnBodyTypeChanged.Broadcast(NewBodyType);
    }
}

void UMetaHumanCharacterComponent::SetGender(EMetaHumanGender NewGender)
{
    if (CurrentGender != NewGender)
    {
        CurrentGender = NewGender;
        if (bIsMetaHumanActive)
        {
            ApplyGenderChanges();
        }
        OnGenderChanged.Broadcast(NewGender);
    }
}

void UMetaHumanCharacterComponent::SetAgeRange(EMetaHumanAgeRange NewAgeRange)
{
    if (CurrentAgeRange != NewAgeRange)
    {
        CurrentAgeRange = NewAgeRange;
        if (bIsMetaHumanActive)
        {
            ApplyAgeChanges();
        }
        OnAgeChanged.Broadcast(NewAgeRange);
    }
}

void UMetaHumanCharacterComponent::ApplyFacialExpression(EFacialExpression Expression, float Intensity)
{
    if (!bIsMetaHumanActive || !bUseProceduralFacialAnimation) return;
    
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    CurrentFacialExpression = Expression;
    FacialExpressionIntensity = Intensity;
    
    // Apply the facial expression through morph targets or control rig
    ApplyFacialMorphTargets(Expression, Intensity);
    
    OnFacialExpressionChanged.Broadcast(Expression, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Applied facial expression: %d with intensity: %f"), 
           static_cast<int32>(Expression), Intensity);
}

void UMetaHumanCharacterComponent::TriggerEmotionalResponse(EEmotionalTrigger Trigger, float Intensity)
{
    if (!bIsMetaHumanActive) return;
    
    // Map emotional triggers to facial expressions
    EFacialExpression TargetExpression = EFacialExpression::Neutral;
    float ExpressionIntensity = Intensity * EmotionalResponseMultiplier;
    
    switch (Trigger)
    {
        case EEmotionalTrigger::DinosaurSighting:
            TargetExpression = EFacialExpression::Surprised;
            ExpressionIntensity *= 0.8f;
            break;
            
        case EEmotionalTrigger::PredatorNearby:
            TargetExpression = EFacialExpression::Fearful;
            ExpressionIntensity *= 1.2f;
            break;
            
        case EEmotionalTrigger::SafeAreaReached:
            TargetExpression = EFacialExpression::Happy;
            ExpressionIntensity *= 0.6f;
            break;
            
        case EEmotionalTrigger::FoodFound:
            TargetExpression = EFacialExpression::Happy;
            ExpressionIntensity *= 0.7f;
            break;
            
        case EEmotionalTrigger::LowHealth:
            TargetExpression = EFacialExpression::Pained;
            ExpressionIntensity *= 0.9f;
            break;
    }
    
    ApplyFacialExpression(TargetExpression, ExpressionIntensity);
}

void UMetaHumanCharacterComponent::UpdateClothingMaterial(int32 MaterialIndex, UMaterialInterface* NewMaterial)
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp || MaterialIndex >= MeshComp->GetNumMaterials()) return;
    
    MeshComp->SetMaterial(MaterialIndex, NewMaterial);
    OnClothingChanged.Broadcast(MaterialIndex, NewMaterial);
    
    UE_LOG(LogTemp, Log, TEXT("Updated clothing material at index: %d"), MaterialIndex);
}

void UMetaHumanCharacterComponent::SetLODLevel(int32 LODLevel)
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    LODLevel = FMath::Clamp(LODLevel, 0, MeshComp->GetNumLODs() - 1);
    MeshComp->SetForcedLOD(LODLevel + 1); // UE5 uses 1-based LOD indexing
    
    UE_LOG(LogTemp, Log, TEXT("Set MetaHuman LOD level to: %d"), LODLevel);
}

FMetaHumanConfiguration UMetaHumanCharacterComponent::GetCurrentConfiguration() const
{
    FMetaHumanConfiguration Config;
    Config.BodyType = CurrentBodyType;
    Config.Gender = CurrentGender;
    Config.AgeRange = CurrentAgeRange;
    Config.FacialExpression = CurrentFacialExpression;
    Config.ExpressionIntensity = FacialExpressionIntensity;
    return Config;
}

void UMetaHumanCharacterComponent::ApplyConfiguration(const FMetaHumanConfiguration& Configuration)
{
    CurrentBodyType = Configuration.BodyType;
    CurrentGender = Configuration.Gender;
    CurrentAgeRange = Configuration.AgeRange;
    
    if (bIsMetaHumanActive)
    {
        ApplyMetaHumanConfiguration();
        ApplyFacialExpression(Configuration.FacialExpression, Configuration.ExpressionIntensity);
    }
    
    OnConfigurationChanged.Broadcast(Configuration);
}

void UMetaHumanCharacterComponent::InitializeMetaHumanPresets()
{
    // Initialize body type presets
    BodyTypePresets.Empty();
    BodyTypePresets.Add(EMetaHumanBodyType::Slim, TEXT("Slim body type with lean proportions"));
    BodyTypePresets.Add(EMetaHumanBodyType::Medium, TEXT("Average body type with balanced proportions"));
    BodyTypePresets.Add(EMetaHumanBodyType::Heavy, TEXT("Heavy body type with robust proportions"));
    BodyTypePresets.Add(EMetaHumanBodyType::Athletic, TEXT("Athletic body type with muscular definition"));
    
    // Initialize facial expression mappings
    FacialExpressionMorphTargets.Empty();
    FacialExpressionMorphTargets.Add(EFacialExpression::Neutral, TArray<FString>{TEXT("Neutral_Base")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Happy, TArray<FString>{TEXT("Smile_L"), TEXT("Smile_R"), TEXT("CheekPuff_L"), TEXT("CheekPuff_R")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Sad, TArray<FString>{TEXT("FrownMouth_L"), TEXT("FrownMouth_R"), TEXT("BrowDown_L"), TEXT("BrowDown_R")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Angry, TArray<FString>{TEXT("BrowDown_L"), TEXT("BrowDown_R"), TEXT("NoseWrinkle_L"), TEXT("NoseWrinkle_R")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Surprised, TArray<FString>{TEXT("BrowUp_L"), TEXT("BrowUp_R"), TEXT("EyeWide_L"), TEXT("EyeWide_R")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Fearful, TArray<FString>{TEXT("BrowUp_L"), TEXT("BrowUp_R"), TEXT("EyeWide_L"), TEXT("EyeWide_R"), TEXT("MouthOpen")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Disgusted, TArray<FString>{TEXT("NoseWrinkle_L"), TEXT("NoseWrinkle_R"), TEXT("UpperLipUp_L"), TEXT("UpperLipUp_R")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Contemptuous, TArray<FString>{TEXT("LipCornerPull_L"), TEXT("NoseWrinkle_L")});
    FacialExpressionMorphTargets.Add(EFacialExpression::Pained, TArray<FString>{TEXT("EyeSquint_L"), TEXT("EyeSquint_R"), TEXT("BrowDown_L"), TEXT("BrowDown_R")});
}

void UMetaHumanCharacterComponent::ApplyMetaHumanConfiguration()
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Apply body type scaling
    FVector BodyScale = GetBodyTypeScale(CurrentBodyType);
    Character->SetActorScale3D(BodyScale);
    
    // Apply gender-specific adjustments
    ApplyGenderSpecificSettings();
    
    // Apply age-specific adjustments
    ApplyAgeSpecificSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Applied MetaHuman configuration: BodyType=%d, Gender=%d, Age=%d"), 
           static_cast<int32>(CurrentBodyType), static_cast<int32>(CurrentGender), static_cast<int32>(CurrentAgeRange));
}

void UMetaHumanCharacterComponent::ApplyBodyTypeChanges()
{
    if (!GetOwner()) return;
    
    FVector NewScale = GetBodyTypeScale(CurrentBodyType);
    GetOwner()->SetActorScale3D(NewScale);
}

void UMetaHumanCharacterComponent::ApplyGenderChanges()
{
    ApplyGenderSpecificSettings();
}

void UMetaHumanCharacterComponent::ApplyAgeChanges()
{
    ApplyAgeSpecificSettings();
}

void UMetaHumanCharacterComponent::SetupFacialAnimationSystem()
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Initialize morph target values
    for (const auto& ExpressionPair : FacialExpressionMorphTargets)
    {
        for (const FString& MorphTargetName : ExpressionPair.Value)
        {
            MeshComp->SetMorphTarget(FName(*MorphTargetName), 0.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Facial animation system setup complete"));
}

void UMetaHumanCharacterComponent::SetupLODSystem()
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Configure LOD settings for performance
    MeshComp->bAllowAnimCurveEvaluation = true;
    MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    
    UE_LOG(LogTemp, Log, TEXT("LOD system setup complete"));
}

void UMetaHumanCharacterComponent::UpdateFacialExpressions(float DeltaTime)
{
    // Gradually blend facial expressions back to neutral if no active expression
    if (CurrentFacialExpression != EFacialExpression::Neutral && FacialExpressionIntensity > 0.0f)
    {
        FacialExpressionIntensity = FMath::Max(0.0f, FacialExpressionIntensity - (DeltaTime * 0.5f));
        
        if (FacialExpressionIntensity <= 0.0f)
        {
            CurrentFacialExpression = EFacialExpression::Neutral;
        }
        
        ApplyFacialMorphTargets(CurrentFacialExpression, FacialExpressionIntensity);
    }
}

void UMetaHumanCharacterComponent::UpdateLODSystem(float DeltaTime)
{
    if (!bUseLODSystem || !GetOwner()) return;
    
    // Automatic LOD management based on distance to player
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
            
            int32 TargetLOD = 0;
            if (Distance > 2000.0f) TargetLOD = 3;
            else if (Distance > 1000.0f) TargetLOD = 2;
            else if (Distance > 500.0f) TargetLOD = 1;
            
            SetLODLevel(TargetLOD);
        }
    }
}

void UMetaHumanCharacterComponent::ApplyFacialMorphTargets(EFacialExpression Expression, float Intensity)
{
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Reset all morph targets first
    for (const auto& ExpressionPair : FacialExpressionMorphTargets)
    {
        for (const FString& MorphTargetName : ExpressionPair.Value)
        {
            MeshComp->SetMorphTarget(FName(*MorphTargetName), 0.0f);
        }
    }
    
    // Apply current expression morph targets
    if (FacialExpressionMorphTargets.Contains(Expression))
    {
        const TArray<FString>& MorphTargets = FacialExpressionMorphTargets[Expression];
        for (const FString& MorphTargetName : MorphTargets)
        {
            MeshComp->SetMorphTarget(FName(*MorphTargetName), Intensity);
        }
    }
}

FVector UMetaHumanCharacterComponent::GetBodyTypeScale(EMetaHumanBodyType BodyType) const
{
    switch (BodyType)
    {
        case EMetaHumanBodyType::Slim:
            return FVector(0.9f, 0.9f, 1.0f);
        case EMetaHumanBodyType::Medium:
            return FVector(1.0f, 1.0f, 1.0f);
        case EMetaHumanBodyType::Heavy:
            return FVector(1.2f, 1.2f, 1.0f);
        case EMetaHumanBodyType::Athletic:
            return FVector(1.1f, 1.1f, 1.05f);
        default:
            return FVector(1.0f, 1.0f, 1.0f);
    }
}

void UMetaHumanCharacterComponent::ApplyGenderSpecificSettings()
{
    // Apply gender-specific material parameters or mesh adjustments
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Example: Apply different voice pitch or animation sets based on gender
    float VoicePitch = (CurrentGender == EMetaHumanGender::Female) ? 1.2f : 0.8f;
    
    // Apply gender-specific material parameters
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
    {
        if (UMaterialInterface* Material = MeshComp->GetMaterial(i))
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("VoicePitch"), VoicePitch);
                MeshComp->SetMaterial(i, DynamicMaterial);
            }
        }
    }
}

void UMetaHumanCharacterComponent::ApplyAgeSpecificSettings()
{
    // Apply age-specific adjustments like skin texture, posture, etc.
    if (!GetOwner()) return;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    float AgeMultiplier = 1.0f;
    switch (CurrentAgeRange)
    {
        case EMetaHumanAgeRange::Young:
            AgeMultiplier = 0.8f;
            break;
        case EMetaHumanAgeRange::Adult:
            AgeMultiplier = 1.0f;
            break;
        case EMetaHumanAgeRange::MiddleAged:
            AgeMultiplier = 1.1f;
            break;
        case EMetaHumanAgeRange::Elderly:
            AgeMultiplier = 1.2f;
            break;
    }
    
    // Apply age-specific material parameters
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
    {
        if (UMaterialInterface* Material = MeshComp->GetMaterial(i))
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("AgeMultiplier"), AgeMultiplier);
                DynamicMaterial->SetScalarParameterValue(TEXT("SkinRoughness"), AgeMultiplier * 0.5f);
                MeshComp->SetMaterial(i, DynamicMaterial);
            }
        }
    }
}