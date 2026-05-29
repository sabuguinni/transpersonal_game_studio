#include "TribalCharacterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

UTribalCharacterComponent::UTribalCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Initialize default appearance
    Appearance.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    Appearance.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    Appearance.ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    Appearance.ScarIntensity = 0.5f;
    Appearance.WeatheringLevel = 0.7f;
    Appearance.bHasFacePaint = true;
    Appearance.bHasTribalMarkings = true;
}

void UTribalCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultSkills();
    ApplyTribalAppearance();
}

void UTribalCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTribalCharacterComponent::ApplyTribalAppearance()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterComponent: No skeletal mesh component found"));
        return;
    }

    // Create dynamic material instance if not already created
    if (!DynamicMaterial && MeshComp->GetMaterial(0))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(MeshComp->GetMaterial(0), this);
        if (DynamicMaterial)
        {
            MeshComp->SetMaterial(0, DynamicMaterial);
        }
    }

    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    InitializeDefaultSkills();
    
    // Adjust appearance based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            Appearance.WeatheringLevel = 0.8f;
            Appearance.ScarIntensity = 0.7f;
            break;
        case EChar_TribalRole::Elder:
            Appearance.WeatheringLevel = 0.9f;
            Appearance.HairColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray hair
            break;
        case EChar_TribalRole::Shaman:
            Appearance.bHasFacePaint = true;
            Appearance.bHasTribalMarkings = true;
            break;
        case EChar_TribalRole::Gatherer:
            Appearance.WeatheringLevel = 0.6f;
            Appearance.ScarIntensity = 0.3f;
            break;
        default:
            break;
    }
    
    ApplyTribalAppearance();
}

void UTribalCharacterComponent::RandomizeAppearance()
{
    // Randomize skin tone
    float SkinVariation = FMath::RandRange(0.6f, 1.0f);
    Appearance.SkinTone = FLinearColor(
        SkinVariation * 0.8f,
        SkinVariation * 0.6f,
        SkinVariation * 0.4f,
        1.0f
    );

    // Randomize hair color
    float HairDarkness = FMath::RandRange(0.05f, 0.4f);
    Appearance.HairColor = FLinearColor(HairDarkness, HairDarkness * 0.8f, HairDarkness * 0.5f, 1.0f);

    // Randomize weathering and scars
    Appearance.WeatheringLevel = FMath::RandRange(0.4f, 0.9f);
    Appearance.ScarIntensity = FMath::RandRange(0.2f, 0.8f);

    // Random chance for face paint and markings
    Appearance.bHasFacePaint = FMath::RandBool();
    Appearance.bHasTribalMarkings = FMath::RandBool();

    ApplyTribalAppearance();
}

void UTribalCharacterComponent::ApplyWeathering(float WeatheringAmount)
{
    Appearance.WeatheringLevel = FMath::Clamp(Appearance.WeatheringLevel + WeatheringAmount, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

FString UTribalCharacterComponent::GetRoleDescription() const
{
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            return TEXT("Skilled in tracking and hunting prehistoric beasts");
        case EChar_TribalRole::Gatherer:
            return TEXT("Expert in finding edible plants and resources");
        case EChar_TribalRole::Elder:
            return TEXT("Wise leader with knowledge of survival");
        case EChar_TribalRole::Shaman:
            return TEXT("Spiritual guide and healer of the tribe");
        case EChar_TribalRole::Warrior:
            return TEXT("Fierce protector against dinosaur threats");
        case EChar_TribalRole::Crafter:
            return TEXT("Master of tools and primitive technology");
        default:
            return TEXT("Member of the prehistoric tribe");
    }
}

void UTribalCharacterComponent::InitializeDefaultSkills()
{
    Skills.Empty();
    
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            Skills.AddUnique(TEXT("Tracking"));
            Skills.AddUnique(TEXT("Spear Combat"));
            Skills.AddUnique(TEXT("Stealth"));
            break;
        case EChar_TribalRole::Gatherer:
            Skills.AddUnique(TEXT("Plant Knowledge"));
            Skills.AddUnique(TEXT("Resource Finding"));
            Skills.AddUnique(TEXT("Medicine"));
            break;
        case EChar_TribalRole::Elder:
            Skills.AddUnique(TEXT("Leadership"));
            Skills.AddUnique(TEXT("Wisdom"));
            Skills.AddUnique(TEXT("Storytelling"));
            break;
        case EChar_TribalRole::Shaman:
            Skills.AddUnique(TEXT("Healing"));
            Skills.AddUnique(TEXT("Ritual Knowledge"));
            Skills.AddUnique(TEXT("Animal Communication"));
            break;
        case EChar_TribalRole::Warrior:
            Skills.AddUnique(TEXT("Melee Combat"));
            Skills.AddUnique(TEXT("Defense"));
            Skills.AddUnique(TEXT("Intimidation"));
            break;
        case EChar_TribalRole::Crafter:
            Skills.AddUnique(TEXT("Tool Making"));
            Skills.AddUnique(TEXT("Fire Starting"));
            Skills.AddUnique(TEXT("Construction"));
            break;
    }
}

void UTribalCharacterComponent::UpdateMaterialParameters()
{
    if (!DynamicMaterial)
    {
        return;
    }

    // Set material parameters for tribal appearance
    DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), Appearance.SkinTone);
    DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), Appearance.HairColor);
    DynamicMaterial->SetVectorParameterValue(TEXT("ClothingColor"), Appearance.ClothingColor);
    DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Appearance.ScarIntensity);
    DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), Appearance.WeatheringLevel);
    DynamicMaterial->SetScalarParameterValue(TEXT("FacePaint"), Appearance.bHasFacePaint ? 1.0f : 0.0f);
    DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), Appearance.bHasTribalMarkings ? 1.0f : 0.0f);
}

USkeletalMeshComponent* UTribalCharacterComponent::GetCharacterMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    
    // Fallback: search for skeletal mesh component
    return GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
}