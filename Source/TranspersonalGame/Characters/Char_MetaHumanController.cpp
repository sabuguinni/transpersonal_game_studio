#include "Char_MetaHumanController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UChar_MetaHumanController::UChar_MetaHumanController()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default character variation
    CharacterVariation.CharacterType = EChar_CharacterType::Player;
    CharacterVariation.AgeGroup = EChar_AgeGroup::YoungAdult;
    CharacterVariation.ScaleModifier = FVector(1.0f, 1.0f, 1.0f);
    CharacterVariation.CharacterName = TEXT("Tribal Survivor");
    CharacterVariation.CharacterDescription = FText::FromString(TEXT("A resilient human adapting to survive in the dangerous Cretaceous world"));
    
    bAutoApplyVariation = true;
    bEnableSubsurfaceScattering = true;
    bEnableDynamicShadows = true;
    SkinSubsurfaceIntensity = 1.0f;
    SkinSubsurfaceColor = FLinearColor(1.0f, 0.4f, 0.3f, 1.0f);
}

void UChar_MetaHumanController::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoApplyVariation)
    {
        ApplyCharacterVariation(CharacterVariation);
    }
    
    // Optimize for fire lighting by default
    OptimizeForFireLighting();
}

void UChar_MetaHumanController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_MetaHumanController::ApplyCharacterVariation(const FChar_CharacterVariation& NewVariation)
{
    CharacterVariation = NewVariation;
    
    UpdateCharacterMesh();
    UpdateCharacterMaterials();
    UpdateLightingConfiguration();
    
    // Apply scale modification
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorScale3D(CharacterVariation.ScaleModifier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied character variation: %s"), *CharacterVariation.CharacterName);
}

void UChar_MetaHumanController::SetCharacterType(EChar_CharacterType NewType)
{
    CharacterVariation.CharacterType = NewType;
    
    // Update character properties based on type
    switch (NewType)
    {
        case EChar_CharacterType::TribalElder:
            CharacterVariation.ScaleModifier = FVector(1.1f, 1.1f, 1.1f);
            CharacterVariation.CharacterName = TEXT("Tribal Elder");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Wise leader of the tribe with extensive survival knowledge"));
            break;
            
        case EChar_CharacterType::Gatherer:
            CharacterVariation.ScaleModifier = FVector(0.95f, 0.95f, 1.0f);
            CharacterVariation.CharacterName = TEXT("Gatherer");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Skilled in finding and collecting edible plants and resources"));
            break;
            
        case EChar_CharacterType::Hunter:
            CharacterVariation.ScaleModifier = FVector(1.05f, 1.05f, 1.0f);
            CharacterVariation.CharacterName = TEXT("Hunter");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Expert tracker and hunter of prehistoric creatures"));
            break;
            
        case EChar_CharacterType::Scout:
            CharacterVariation.ScaleModifier = FVector(0.9f, 0.9f, 1.0f);
            CharacterVariation.CharacterName = TEXT("Scout");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Fast and agile explorer who surveys dangerous territories"));
            break;
            
        case EChar_CharacterType::Warrior:
            CharacterVariation.ScaleModifier = FVector(1.15f, 1.15f, 1.05f);
            CharacterVariation.CharacterName = TEXT("Warrior");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Fierce defender of the tribe against dinosaur threats"));
            break;
            
        case EChar_CharacterType::Shaman:
            CharacterVariation.ScaleModifier = FVector(1.0f, 1.0f, 1.0f);
            CharacterVariation.CharacterName = TEXT("Tribal Healer");
            CharacterVariation.CharacterDescription = FText::FromString(TEXT("Knowledgeable in herbal medicine and tribal customs"));
            break;
            
        default:
            CharacterVariation.ScaleModifier = FVector(1.0f, 1.0f, 1.0f);
            CharacterVariation.CharacterName = TEXT("Tribal Survivor");
            break;
    }
    
    ApplyCharacterVariation(CharacterVariation);
}

void UChar_MetaHumanController::SetAgeGroup(EChar_AgeGroup NewAgeGroup)
{
    CharacterVariation.AgeGroup = NewAgeGroup;
    
    // Adjust scale based on age
    FVector BaseScale = CharacterVariation.ScaleModifier;
    
    switch (NewAgeGroup)
    {
        case EChar_AgeGroup::Child:
            CharacterVariation.ScaleModifier = BaseScale * 0.7f;
            break;
        case EChar_AgeGroup::Teenager:
            CharacterVariation.ScaleModifier = BaseScale * 0.85f;
            break;
        case EChar_AgeGroup::YoungAdult:
            CharacterVariation.ScaleModifier = BaseScale * 1.0f;
            break;
        case EChar_AgeGroup::MiddleAged:
            CharacterVariation.ScaleModifier = BaseScale * 1.05f;
            break;
        case EChar_AgeGroup::Elder:
            CharacterVariation.ScaleModifier = BaseScale * 0.95f;
            break;
    }
    
    ApplyCharacterVariation(CharacterVariation);
}

void UChar_MetaHumanController::OptimizeForFireLighting()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Enable subsurface scattering for realistic skin lighting
    if (bEnableSubsurfaceScattering)
    {
        // Create dynamic material instances for subsurface scattering
        for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
        {
            UMaterialInterface* Material = MeshComp->GetMaterial(i);
            if (Material)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
                if (DynamicMaterial)
                {
                    // Set subsurface parameters for fire lighting
                    DynamicMaterial->SetScalarParameterValue(TEXT("SubsurfaceIntensity"), SkinSubsurfaceIntensity);
                    DynamicMaterial->SetVectorParameterValue(TEXT("SubsurfaceColor"), SkinSubsurfaceColor);
                    
                    MeshComp->SetMaterial(i, DynamicMaterial);
                }
            }
        }
    }
    
    // Configure shadow casting for fire lighting
    ConfigureShadowCasting(bEnableDynamicShadows, true);
    
    UE_LOG(LogTemp, Log, TEXT("Optimized character lighting for fire environments"));
}

void UChar_MetaHumanController::ConfigureShadowCasting(bool bCastDynamicShadows, bool bCastStaticShadows)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    MeshComp->SetCastShadow(bCastDynamicShadows || bCastStaticShadows);
    MeshComp->bCastDynamicShadow = bCastDynamicShadows;
    MeshComp->bCastStaticShadow = bCastStaticShadows;
    MeshComp->bReceivesDecals = true;
    
    UE_LOG(LogTemp, Log, TEXT("Configured shadow casting - Dynamic: %s, Static: %s"), 
           bCastDynamicShadows ? TEXT("True") : TEXT("False"),
           bCastStaticShadows ? TEXT("True") : TEXT("False"));
}

void UChar_MetaHumanController::UpdateCharacterMesh()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp || !CharacterVariation.CharacterMesh)
    {
        return;
    }
    
    MeshComp->SetSkeletalMesh(CharacterVariation.CharacterMesh);
    UE_LOG(LogTemp, Log, TEXT("Updated character mesh for %s"), *CharacterVariation.CharacterName);
}

void UChar_MetaHumanController::UpdateCharacterMaterials()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Apply character-specific materials
    for (int32 i = 0; i < CharacterVariation.CharacterMaterials.Num() && i < MeshComp->GetNumMaterials(); ++i)
    {
        if (CharacterVariation.CharacterMaterials[i])
        {
            MeshComp->SetMaterial(i, CharacterVariation.CharacterMaterials[i]);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated character materials for %s"), *CharacterVariation.CharacterName);
}

void UChar_MetaHumanController::UpdateLightingConfiguration()
{
    OptimizeForFireLighting();
}