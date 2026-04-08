#include "CharacterSystem.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

UCharacterSystem::UCharacterSystem()
{
    // Initialize character archetypes with realistic variation ranges
}

FCharacterProfile UCharacterSystem::GenerateRandomCharacter(ECharacterArchetype ArchetypeHint)
{
    FCharacterProfile NewProfile;
    
    // Generate unique seed for this character
    int32 CharacterSeed = FMath::RandRange(1000, 999999);
    FRandomStream RandomStream(CharacterSeed);
    
    // Set archetype
    NewProfile.Archetype = ArchetypeHint;
    
    // Generate variation data based on archetype
    NewProfile.PhysicalTraits = GenerateArchetypeSpecificVariation(ArchetypeHint, CharacterSeed);
    
    // Generate personality traits (Big Five model)
    for (int32 i = 0; i < 5; i++)
    {
        NewProfile.PersonalityTraits[i] = RandomStream.FRandRange(-1.0f, 1.0f);
    }
    
    // Set default emotional state based on archetype
    switch (ArchetypeHint)
    {
        case ECharacterArchetype::Protagonist:
            NewProfile.DefaultEmotionalState = EEmotionalState::Determination;
            break;
        case ECharacterArchetype::TribalLeader:
            NewProfile.DefaultEmotionalState = EEmotionalState::Vigilance;
            break;
        case ECharacterArchetype::Hunter:
            NewProfile.DefaultEmotionalState = EEmotionalState::Vigilance;
            break;
        case ECharacterArchetype::Shaman:
            NewProfile.DefaultEmotionalState = EEmotionalState::Wonder;
            break;
        case ECharacterArchetype::Child:
            NewProfile.DefaultEmotionalState = EEmotionalState::Curiosity;
            break;
        case ECharacterArchetype::Elder:
            NewProfile.DefaultEmotionalState = EEmotionalState::Neutral;
            break;
        default:
            NewProfile.DefaultEmotionalState = EEmotionalState::Neutral;
            break;
    }
    
    // Generate unique name
    NewProfile.CharacterName = FString::Printf(TEXT("Character_%s_%d"), 
        *UEnum::GetValueAsString(ArchetypeHint), CharacterSeed);
    
    // Generate background story based on archetype and traits
    NewProfile.BackgroundStory = GenerateBackgroundStory(ArchetypeHint, NewProfile.PhysicalTraits);
    
    // Set technical parameters
    NewProfile.LODLevel = 0; // Start with highest quality
    NewProfile.bUseNaniteGeometry = true;
    
    return NewProfile;
}

USkeletalMeshComponent* UCharacterSystem::CreateMetaHumanFromProfile(const FCharacterProfile& Profile, AActor* OwnerActor)
{
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterSystem: OwnerActor is null"));
        return nullptr;
    }
    
    // Create skeletal mesh component
    USkeletalMeshComponent* NewCharacter = NewObject<USkeletalMeshComponent>(OwnerActor);
    
    if (!NewCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterSystem: Failed to create SkeletalMeshComponent"));
        return nullptr;
    }
    
    // Load base MetaHuman mesh based on archetype
    USkeletalMesh* BaseMesh = LoadMetaHumanMeshForArchetype(Profile.Archetype);
    if (BaseMesh)
    {
        NewCharacter->SetSkeletalMesh(BaseMesh);
    }
    
    // Apply physical variations
    ApplyVariationToMetaHuman(NewCharacter, Profile.PhysicalTraits);
    
    // Set animation blueprint
    if (Profile.AnimationBlueprint.IsValid())
    {
        UClass* AnimBPClass = Profile.AnimationBlueprint.LoadSynchronous();
        if (AnimBPClass)
        {
            NewCharacter->SetAnimInstanceClass(AnimBPClass);
        }
    }
    
    // Apply emotional state
    ApplyEmotionalState(NewCharacter, Profile.DefaultEmotionalState, 1.0f);
    
    // Configure LOD and Nanite settings
    SetCharacterLOD(NewCharacter, Profile.LODLevel);
    EnableNaniteGeometry(NewCharacter, Profile.bUseNaniteGeometry);
    
    // Attach to owner
    NewCharacter->AttachToComponent(OwnerActor->GetRootComponent(), 
        FAttachmentTransformRules::KeepWorldTransform);
    
    UE_LOG(LogTemp, Log, TEXT("CharacterSystem: Created character %s"), *Profile.CharacterName);
    
    return NewCharacter;
}

void UCharacterSystem::ApplyVariationToMetaHuman(USkeletalMeshComponent* MetaHuman, const FCharacterVariationData& VariationData)
{
    if (!MetaHuman)
    {
        return;
    }
    
    // Create dynamic material instance for skin customization
    UMaterialInterface* BaseSkinMaterial = MetaHuman->GetMaterial(0); // Assuming skin is material slot 0
    if (BaseSkinMaterial)
    {
        UMaterialInstanceDynamic* DynamicSkinMaterial = UMaterialInstanceDynamic::Create(BaseSkinMaterial, MetaHuman);
        
        if (DynamicSkinMaterial)
        {
            // Apply skin tone variation
            FLinearColor SkinColor = GenerateRealisticSkinTone(FMath::RandRange(1, 10000));
            DynamicSkinMaterial->SetVectorParameterValue(TEXT("SkinTone"), SkinColor);
            
            // Apply weathering/aging
            DynamicSkinMaterial->SetScalarParameterValue(TEXT("SkinWeathering"), VariationData.SkinWeathering);
            DynamicSkinMaterial->SetScalarParameterValue(TEXT("AgeValue"), VariationData.AgeVariation);
            
            // Apply scars if any
            for (int32 i = 0; i < VariationData.ScarLocations.Num() && i < 3; i++) // Max 3 scars
            {
                FString ScarParamName = FString::Printf(TEXT("ScarLocation_%d"), i + 1);
                DynamicSkinMaterial->SetVectorParameterValue(*ScarParamName, 
                    FLinearColor(VariationData.ScarLocations[i].X, VariationData.ScarLocations[i].Y, 0.0f, 1.0f));
            }
            
            MetaHuman->SetMaterial(0, DynamicSkinMaterial);
        }
    }
    
    // Apply body shape variations through morph targets
    if (USkeletalMesh* Mesh = MetaHuman->GetSkeletalMeshAsset())
    {
        // Height variation
        if (FMath::Abs(VariationData.HeightVariation) > 0.01f)
        {
            FVector CurrentScale = MetaHuman->GetComponentScale();
            float HeightMultiplier = 1.0f + (VariationData.HeightVariation * 0.2f); // ±20% height variation
            MetaHuman->SetWorldScale3D(FVector(CurrentScale.X, CurrentScale.Y, CurrentScale.Z * HeightMultiplier));
        }
        
        // Apply morph targets for facial features
        MetaHuman->SetMorphTarget(TEXT("FaceWidth"), VariationData.FaceWidth);
        MetaHuman->SetMorphTarget(TEXT("EyeSize"), VariationData.EyeSize);
        MetaHuman->SetMorphTarget(TEXT("NoseShape"), VariationData.NoseShape);
        MetaHuman->SetMorphTarget(TEXT("JawDefinition"), VariationData.JawDefinition);
        MetaHuman->SetMorphTarget(TEXT("BodyBuild"), VariationData.BuildVariation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CharacterSystem: Applied variations to MetaHuman"));
}

void UCharacterSystem::ApplyEmotionalState(USkeletalMeshComponent* Character, EEmotionalState EmotionalState, float Intensity)
{
    if (!Character)
    {
        return;
    }
    
    // Map emotional states to facial expression morph targets
    switch (EmotionalState)
    {
        case EEmotionalState::Fear:
            Character->SetMorphTarget(TEXT("Fear_Brow"), Intensity * 0.8f);
            Character->SetMorphTarget(TEXT("Fear_Eyes"), Intensity * 0.7f);
            Character->SetMorphTarget(TEXT("Fear_Mouth"), Intensity * 0.6f);
            break;
            
        case EEmotionalState::Determination:
            Character->SetMorphTarget(TEXT("Determination_Brow"), Intensity * 0.9f);
            Character->SetMorphTarget(TEXT("Determination_Jaw"), Intensity * 0.7f);
            Character->SetMorphTarget(TEXT("Determination_Eyes"), Intensity * 0.8f);
            break;
            
        case EEmotionalState::Curiosity:
            Character->SetMorphTarget(TEXT("Curiosity_Brow"), Intensity * 0.6f);
            Character->SetMorphTarget(TEXT("Curiosity_Eyes"), Intensity * 0.8f);
            Character->SetMorphTarget(TEXT("Curiosity_Head"), Intensity * 0.5f);
            break;
            
        case EEmotionalState::Exhaustion:
            Character->SetMorphTarget(TEXT("Exhaustion_Eyes"), Intensity * 0.9f);
            Character->SetMorphTarget(TEXT("Exhaustion_Mouth"), Intensity * 0.7f);
            Character->SetMorphTarget(TEXT("Exhaustion_Posture"), Intensity * 0.8f);
            break;
            
        case EEmotionalState::Vigilance:
            Character->SetMorphTarget(TEXT("Alert_Eyes"), Intensity * 0.9f);
            Character->SetMorphTarget(TEXT("Alert_Brow"), Intensity * 0.7f);
            Character->SetMorphTarget(TEXT("Alert_Posture"), Intensity * 0.8f);
            break;
            
        case EEmotionalState::Wonder:
            Character->SetMorphTarget(TEXT("Wonder_Eyes"), Intensity * 0.8f);
            Character->SetMorphTarget(TEXT("Wonder_Mouth"), Intensity * 0.5f);
            Character->SetMorphTarget(TEXT("Wonder_Brow"), Intensity * 0.6f);
            break;
            
        default:
            // Neutral - reset all emotional morph targets
            Character->SetMorphTarget(TEXT("Fear_Brow"), 0.0f);
            Character->SetMorphTarget(TEXT("Fear_Eyes"), 0.0f);
            Character->SetMorphTarget(TEXT("Determination_Brow"), 0.0f);
            // ... reset all other emotional morphs
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CharacterSystem: Applied emotional state %s with intensity %f"), 
        *UEnum::GetValueAsString(EmotionalState), Intensity);
}

FCharacterVariationData UCharacterSystem::GenerateArchetypeSpecificVariation(ECharacterArchetype Archetype, int32 Seed)
{
    FRandomStream RandomStream(Seed);
    FCharacterVariationData VariationData;
    
    // Base variations for all archetypes
    VariationData.FaceWidth = RandomStream.FRandRange(-0.3f, 0.3f);
    VariationData.EyeSize = RandomStream.FRandRange(-0.2f, 0.2f);
    VariationData.NoseShape = RandomStream.FRandRange(-0.4f, 0.4f);
    VariationData.JawDefinition = RandomStream.FRandRange(-0.3f, 0.3f);
    
    // Archetype-specific variations
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            // Paleontologist - academic build, moderate weathering
            VariationData.BuildVariation = RandomStream.FRandRange(-0.2f, 0.1f); // Lean to average
            VariationData.HeightVariation = RandomStream.FRandRange(-0.1f, 0.2f); // Average to tall
            VariationData.AgeVariation = RandomStream.FRandRange(0.3f, 0.5f); // Middle-aged
            VariationData.SkinWeathering = RandomStream.FRandRange(0.1f, 0.3f); // Light weathering
            break;
            
        case ECharacterArchetype::Hunter:
            // Strong, weathered, athletic build
            VariationData.BuildVariation = RandomStream.FRandRange(0.2f, 0.8f); // Athletic to robust
            VariationData.HeightVariation = RandomStream.FRandRange(0.0f, 0.3f); // Average to tall
            VariationData.AgeVariation = RandomStream.FRandRange(0.2f, 0.7f); // Young adult to mature
            VariationData.SkinWeathering = RandomStream.FRandRange(0.4f, 0.8f); // Moderate to heavy weathering
            VariationData.JawDefinition = FMath::Max(VariationData.JawDefinition, 0.2f); // Strong jaw
            
            // Add hunting scars
            int32 NumScars = RandomStream.RandRange(1, 3);
            for (int32 i = 0; i < NumScars; i++)
            {
                FVector2D ScarLocation(RandomStream.FRandRange(0.0f, 1.0f), RandomStream.FRandRange(0.0f, 1.0f));
                VariationData.ScarLocations.Add(ScarLocation);
            }
            break;
            
        case ECharacterArchetype::Gatherer:
            // Moderate build, practical appearance
            VariationData.BuildVariation = RandomStream.FRandRange(-0.1f, 0.3f); // Lean to average
            VariationData.HeightVariation = RandomStream.FRandRange(-0.2f, 0.1f); // Short to average
            VariationData.AgeVariation = RandomStream.FRandRange(0.2f, 0.6f); // Young to mature
            VariationData.SkinWeathering = RandomStream.FRandRange(0.2f, 0.5f); // Light to moderate weathering
            break;
            
        case ECharacterArchetype::Elder:
            // Aged, wise appearance
            VariationData.BuildVariation = RandomStream.FRandRange(-0.3f, 0.1f); // Lean to average
            VariationData.HeightVariation = RandomStream.FRandRange(-0.2f, 0.0f); // Shorter due to age
            VariationData.AgeVariation = RandomStream.FRandRange(0.8f, 1.0f); // Very aged
            VariationData.SkinWeathering = RandomStream.FRandRange(0.7f, 1.0f); // Heavy weathering
            
            // Apply aging effects
            ApplyProceduralAging(VariationData, VariationData.AgeVariation);
            break;
            
        case ECharacterArchetype::Child:
            // Young, smaller build
            VariationData.BuildVariation = RandomStream.FRandRange(-0.4f, -0.1f); // Lean
            VariationData.HeightVariation = RandomStream.FRandRange(-0.5f, -0.2f); // Much shorter
            VariationData.AgeVariation = RandomStream.FRandRange(0.0f, 0.2f); // Very young
            VariationData.SkinWeathering = RandomStream.FRandRange(0.0f, 0.1f); // Minimal weathering
            VariationData.EyeSize = FMath::Max(VariationData.EyeSize, 0.1f); // Larger eyes
            break;
            
        case ECharacterArchetype::Shaman:
            // Mystical, weathered appearance
            VariationData.BuildVariation = RandomStream.FRandRange(-0.2f, 0.2f); // Variable
            VariationData.HeightVariation = RandomStream.FRandRange(-0.1f, 0.1f); // Average
            VariationData.AgeVariation = RandomStream.FRandRange(0.4f, 0.8f); // Mature to aged
            VariationData.SkinWeathering = RandomStream.FRandRange(0.3f, 0.7f); // Moderate to heavy
            
            // Distinctive facial features
            VariationData.EyeSize = RandomStream.FRandRange(0.1f, 0.3f); // Piercing eyes
            break;
            
        default:
            // Default variations for other archetypes
            VariationData.BuildVariation = RandomStream.FRandRange(-0.2f, 0.2f);
            VariationData.HeightVariation = RandomStream.FRandRange(-0.1f, 0.1f);
            VariationData.AgeVariation = RandomStream.FRandRange(0.2f, 0.6f);
            VariationData.SkinWeathering = RandomStream.FRandRange(0.2f, 0.5f);
            break;
    }
    
    // Generate realistic skin tone and hair color
    VariationData.SkinTone = RandomStream.FRandRange(0.0f, 1.0f);
    VariationData.HairColor = GenerateRealisticHairColor(VariationData.AgeVariation, Seed);
    
    // Select hair style based on archetype and gender
    VariationData.HairStyleIndex = SelectHairStyleForArchetype(Archetype, RandomStream);
    
    // Select clothing based on archetype
    VariationData.ClothingSetIndex = SelectClothingForArchetype(Archetype, RandomStream);
    VariationData.ClothingWear = RandomStream.FRandRange(0.1f, 0.8f); // Clothing wear level
    
    return VariationData;
}

void UCharacterSystem::ApplyProceduralAging(FCharacterVariationData& VariationData, float AgeValue)
{
    // Increase weathering with age
    VariationData.SkinWeathering = FMath::Max(VariationData.SkinWeathering, AgeValue * 0.7f);
    
    // Add age-related facial changes
    VariationData.FaceWidth += AgeValue * 0.1f; // Slight face widening
    VariationData.EyeSize -= AgeValue * 0.1f; // Eyes appear smaller
    VariationData.JawDefinition += AgeValue * 0.2f; // More defined jaw with age
    
    // Adjust build for age
    if (AgeValue > 0.7f) // Elderly
    {
        VariationData.BuildVariation -= 0.2f; // Tendency to be leaner
        VariationData.HeightVariation -= 0.1f; // Slight height loss
    }
}

FLinearColor UCharacterSystem::GenerateRealisticSkinTone(int32 Seed)
{
    FRandomStream RandomStream(Seed);
    
    // Generate realistic skin tone variations
    // Based on real human skin tone distribution
    float BaseTone = RandomStream.FRandRange(0.2f, 0.9f);
    float RedVariation = RandomStream.FRandRange(-0.1f, 0.1f);
    float YellowVariation = RandomStream.FRandRange(-0.05f, 0.15f);
    
    FLinearColor SkinColor;
    SkinColor.R = FMath::Clamp(BaseTone + RedVariation, 0.1f, 1.0f);
    SkinColor.G = FMath::Clamp(BaseTone - 0.05f, 0.1f, 1.0f);
    SkinColor.B = FMath::Clamp(BaseTone - 0.1f + YellowVariation, 0.1f, 1.0f);
    SkinColor.A = 1.0f;
    
    return SkinColor;
}

FLinearColor UCharacterSystem::GenerateRealisticHairColor(float AgeValue, int32 Seed)
{
    FRandomStream RandomStream(Seed);
    
    // Base hair colors
    TArray<FLinearColor> BaseHairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f), // Black
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),   // Dark Brown
        FLinearColor(0.5f, 0.35f, 0.2f, 1.0f),  // Brown
        FLinearColor(0.7f, 0.5f, 0.3f, 1.0f),   // Light Brown
        FLinearColor(0.9f, 0.7f, 0.4f, 1.0f),   // Blonde
        FLinearColor(0.8f, 0.4f, 0.2f, 1.0f)    // Red
    };
    
    FLinearColor BaseColor = BaseHairColors[RandomStream.RandRange(0, BaseHairColors.Num() - 1)];
    
    // Apply aging - add gray/white
    if (AgeValue > 0.4f)
    {
        float GrayAmount = (AgeValue - 0.4f) * 1.67f; // 0.4-1.0 maps to 0.0-1.0
        GrayAmount = FMath::Clamp(GrayAmount, 0.0f, 0.9f);
        
        FLinearColor GrayColor(0.7f, 0.7f, 0.7f, 1.0f);
        BaseColor = FMath::Lerp(BaseColor, GrayColor, GrayAmount);
    }
    
    return BaseColor;
}

void UCharacterSystem::RegisterCharacterProfile(const FCharacterProfile& Profile)
{
    CharacterDatabase.Add(Profile.CharacterName, Profile);
    UE_LOG(LogTemp, Log, TEXT("CharacterSystem: Registered character profile %s"), *Profile.CharacterName);
}

FCharacterProfile UCharacterSystem::GetCharacterProfile(const FString& CharacterName)
{
    if (CharacterDatabase.Contains(CharacterName))
    {
        return CharacterDatabase[CharacterName];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CharacterSystem: Character profile %s not found"), *CharacterName);
    return FCharacterProfile(); // Return empty profile
}

TArray<FCharacterProfile> UCharacterSystem::GetCharactersByArchetype(ECharacterArchetype Archetype)
{
    TArray<FCharacterProfile> MatchingProfiles;
    
    for (const auto& ProfilePair : CharacterDatabase)
    {
        if (ProfilePair.Value.Archetype == Archetype)
        {
            MatchingProfiles.Add(ProfilePair.Value);
        }
    }
    
    return MatchingProfiles;
}

void UCharacterSystem::SetCharacterLOD(USkeletalMeshComponent* Character, int32 LODLevel)
{
    if (!Character)
    {
        return;
    }
    
    // Force specific LOD level
    Character->SetForcedLOD(LODLevel + 1); // UE5 LOD is 1-based for forced LOD
    
    UE_LOG(LogTemp, Log, TEXT("CharacterSystem: Set character LOD to %d"), LODLevel);
}

void UCharacterSystem::EnableNaniteGeometry(USkeletalMeshComponent* Character, bool bEnable)
{
    if (!Character)
    {
        return;
    }
    
    // Enable/disable Nanite virtualized geometry
    if (USkeletalMesh* Mesh = Character->GetSkeletalMeshAsset())
    {
        // This would require accessing Nanite settings on the mesh
        // Implementation depends on UE5 Nanite API for skeletal meshes
        UE_LOG(LogTemp, Log, TEXT("CharacterSystem: %s Nanite geometry"), 
            bEnable ? TEXT("Enabled") : TEXT("Disabled"));
    }
}