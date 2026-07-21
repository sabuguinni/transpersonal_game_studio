#include "Char_TribalWarriorVisuals.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UChar_TribalWarriorVisuals::UChar_TribalWarriorVisuals()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default customization
    CurrentCustomization.ArmorType = EChar_WarriorArmorType::BoneArmor;
    CurrentCustomization.PrimaryWeapon = EChar_WeaponType::StoneSpear;
    CurrentCustomization.ScarType = EChar_ScarPattern::ClawMarks;
    CurrentCustomization.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    CurrentCustomization.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    CurrentCustomization.MuscleDefinition = 0.8f;
    CurrentCustomization.BattleWear = 0.6f;
    CurrentCustomization.bHasFacialHair = true;
    CurrentCustomization.bHasBoneOrnaments = true;
}

void UChar_TribalWarriorVisuals::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner's skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (OwnerMeshComponent)
        {
            InitializeMaterialInstances();
            ApplyCustomization(CurrentCustomization);
        }
    }
}

void UChar_TribalWarriorVisuals::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_TribalWarriorVisuals::ApplyCustomization(const FChar_WarriorCustomization& NewCustomization)
{
    CurrentCustomization = NewCustomization;
    
    if (!OwnerMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorVisuals: No mesh component found"));
        return;
    }
    
    UpdateMeshMaterials();
    ApplyArmorVariant();
    AttachWeaponMesh();
    ApplyScarPattern();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal warrior customization"));
}

void UChar_TribalWarriorVisuals::RandomizeAppearance()
{
    FChar_WarriorCustomization RandomCustomization;
    
    // Randomize armor type
    int32 ArmorTypeCount = static_cast<int32>(EChar_WarriorArmorType::CompositeArmor) + 1;
    RandomCustomization.ArmorType = static_cast<EChar_WarriorArmorType>(
        UKismetMathLibrary::RandomIntegerInRange(0, ArmorTypeCount - 1)
    );
    
    // Randomize weapon
    int32 WeaponTypeCount = static_cast<int32>(EChar_WeaponType::Sling) + 1;
    RandomCustomization.PrimaryWeapon = static_cast<EChar_WeaponType>(
        UKismetMathLibrary::RandomIntegerInRange(0, WeaponTypeCount - 1)
    );
    
    // Randomize scars
    RandomCustomization.ScarType = SelectRandomScarPattern();
    
    // Randomize skin tone
    RandomCustomization.SkinTone = GenerateRandomSkinTone();
    
    // Randomize hair color (darker tones)
    RandomCustomization.HairColor = FLinearColor(
        UKismetMathLibrary::RandomFloatInRange(0.1f, 0.3f),
        UKismetMathLibrary::RandomFloatInRange(0.05f, 0.15f),
        UKismetMathLibrary::RandomFloatInRange(0.02f, 0.08f),
        1.0f
    );
    
    // Randomize physical attributes
    RandomCustomization.MuscleDefinition = UKismetMathLibrary::RandomFloatInRange(0.6f, 1.0f);
    RandomCustomization.BattleWear = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.9f);
    RandomCustomization.bHasFacialHair = UKismetMathLibrary::RandomBool();
    RandomCustomization.bHasBoneOrnaments = UKismetMathLibrary::RandomBool();
    
    ApplyCustomization(RandomCustomization);
}

void UChar_TribalWarriorVisuals::SetArmorType(EChar_WarriorArmorType NewArmorType)
{
    CurrentCustomization.ArmorType = NewArmorType;
    ApplyArmorVariant();
}

void UChar_TribalWarriorVisuals::SetWeapon(EChar_WeaponType NewWeapon)
{
    CurrentCustomization.PrimaryWeapon = NewWeapon;
    AttachWeaponMesh();
}

void UChar_TribalWarriorVisuals::AddBattleDamage(float DamageAmount)
{
    CurrentCustomization.BattleWear = FMath::Clamp(
        CurrentCustomization.BattleWear + DamageAmount, 
        0.0f, 
        1.0f
    );
    UpdateMeshMaterials();
}

void UChar_TribalWarriorVisuals::ApplySkinTone(FLinearColor NewSkinTone)
{
    CurrentCustomization.SkinTone = NewSkinTone;
    UpdateMeshMaterials();
}

void UChar_TribalWarriorVisuals::SetMuscleDefinition(float MuscleLevel)
{
    CurrentCustomization.MuscleDefinition = FMath::Clamp(MuscleLevel, 0.0f, 1.0f);
    UpdateMeshMaterials();
}

void UChar_TribalWarriorVisuals::UpdateMeshMaterials()
{
    if (!OwnerMeshComponent || DynamicMaterials.Num() == 0)
    {
        return;
    }
    
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            // Apply skin tone
            DynMat->SetVectorParameterValue(TEXT("SkinColor"), CurrentCustomization.SkinTone);
            
            // Apply hair color
            DynMat->SetVectorParameterValue(TEXT("HairColor"), CurrentCustomization.HairColor);
            
            // Apply muscle definition
            DynMat->SetScalarParameterValue(TEXT("MuscleDefinition"), CurrentCustomization.MuscleDefinition);
            
            // Apply battle wear
            DynMat->SetScalarParameterValue(TEXT("BattleWear"), CurrentCustomization.BattleWear);
            
            // Apply facial hair
            DynMat->SetScalarParameterValue(TEXT("FacialHair"), CurrentCustomization.bHasFacialHair ? 1.0f : 0.0f);
        }
    }
}

void UChar_TribalWarriorVisuals::ApplyArmorVariant()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // Apply armor-specific material parameters
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            float ArmorValue = static_cast<float>(CurrentCustomization.ArmorType) / 5.0f;
            DynMat->SetScalarParameterValue(TEXT("ArmorType"), ArmorValue);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied armor variant: %d"), static_cast<int32>(CurrentCustomization.ArmorType));
}

void UChar_TribalWarriorVisuals::AttachWeaponMesh()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // TODO: Implement weapon attachment logic
    // This would attach weapon meshes to specific sockets on the character
    
    UE_LOG(LogTemp, Log, TEXT("Attached weapon: %d"), static_cast<int32>(CurrentCustomization.PrimaryWeapon));
}

void UChar_TribalWarriorVisuals::ApplyScarPattern()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            float ScarValue = static_cast<float>(CurrentCustomization.ScarType) / 5.0f;
            DynMat->SetScalarParameterValue(TEXT("ScarPattern"), ScarValue);
        }
    }
}

void UChar_TribalWarriorVisuals::InitializeMaterialInstances()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    DynamicMaterials.Empty();
    
    int32 MaterialCount = OwnerMeshComponent->GetNumMaterials();
    for (int32 i = 0; i < MaterialCount; i++)
    {
        UMaterialInterface* BaseMaterial = OwnerMeshComponent->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynMat)
            {
                OwnerMeshComponent->SetMaterial(i, DynMat);
                DynamicMaterials.Add(DynMat);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d dynamic materials"), DynamicMaterials.Num());
}

void UChar_TribalWarriorVisuals::LoadAssetReferences()
{
    // TODO: Load mesh and material assets from content browser
    // This would be implemented to load specific tribal warrior assets
}

FLinearColor UChar_TribalWarriorVisuals::GenerateRandomSkinTone()
{
    // Generate realistic human skin tones
    float BaseR = UKismetMathLibrary::RandomFloatInRange(0.6f, 0.9f);
    float BaseG = UKismetMathLibrary::RandomFloatInRange(0.4f, 0.7f);
    float BaseB = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.6f);
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

EChar_ScarPattern UChar_TribalWarriorVisuals::SelectRandomScarPattern()
{
    int32 ScarTypeCount = static_cast<int32>(EChar_ScarPattern::RitualScars) + 1;
    return static_cast<EChar_ScarPattern>(
        UKismetMathLibrary::RandomIntegerInRange(0, ScarTypeCount - 1)
    );
}