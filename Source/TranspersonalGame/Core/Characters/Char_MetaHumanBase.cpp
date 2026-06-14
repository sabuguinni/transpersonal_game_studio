#include "Char_MetaHumanBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

AChar_MetaHumanBase::AChar_MetaHumanBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize equipment components
    MainWeaponComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainWeapon"));
    MainWeaponComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));

    SecondaryWeaponComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryWeapon"));
    SecondaryWeaponComponent->SetupAttachment(GetMesh(), TEXT("hand_l"));

    ShieldComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shield"));
    ShieldComponent->SetupAttachment(GetMesh(), TEXT("hand_l"));

    AccessoryComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Accessory"));
    AccessoryComponent->SetupAttachment(GetMesh(), TEXT("head"));

    // Initialize equipment slots
    EquipmentSlots.Add(TEXT("MainWeapon"), FChar_EquipmentSlot());
    EquipmentSlots.Add(TEXT("SecondaryWeapon"), FChar_EquipmentSlot());
    EquipmentSlots.Add(TEXT("Shield"), FChar_EquipmentSlot());
    EquipmentSlots.Add(TEXT("Accessory"), FChar_EquipmentSlot());
    EquipmentSlots.Add(TEXT("BackItem"), FChar_EquipmentSlot());
    EquipmentSlots.Add(TEXT("BeltItem"), FChar_EquipmentSlot());

    // Set default appearance
    AppearanceData = FChar_AppearanceData();

    // Initialize material instances
    BodyMaterialInstance = nullptr;
    ClothingMaterialInstance = nullptr;
}

void AChar_MetaHumanBase::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEquipmentComponents();
    UpdateMaterialParameters();
}

void AChar_MetaHumanBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChar_MetaHumanBase::InitializeEquipmentComponents()
{
    // Set up equipment slot socket names
    if (EquipmentSlots.Contains(TEXT("MainWeapon")))
    {
        EquipmentSlots[TEXT("MainWeapon")].SocketName = TEXT("hand_r");
    }
    
    if (EquipmentSlots.Contains(TEXT("SecondaryWeapon")))
    {
        EquipmentSlots[TEXT("SecondaryWeapon")].SocketName = TEXT("hand_l");
    }
    
    if (EquipmentSlots.Contains(TEXT("Shield")))
    {
        EquipmentSlots[TEXT("Shield")].SocketName = TEXT("hand_l");
    }
    
    if (EquipmentSlots.Contains(TEXT("Accessory")))
    {
        EquipmentSlots[TEXT("Accessory")].SocketName = TEXT("head");
    }
    
    if (EquipmentSlots.Contains(TEXT("BackItem")))
    {
        EquipmentSlots[TEXT("BackItem")].SocketName = TEXT("spine_03");
    }
    
    if (EquipmentSlots.Contains(TEXT("BeltItem")))
    {
        EquipmentSlots[TEXT("BeltItem")].SocketName = TEXT("pelvis");
    }
}

void AChar_MetaHumanBase::SetAppearanceData(const FChar_AppearanceData& NewAppearanceData)
{
    AppearanceData = NewAppearanceData;
    UpdateMaterialParameters();
}

void AChar_MetaHumanBase::ApplyTribalMarkings(int32 MarkingType, FLinearColor Color)
{
    AppearanceData.TribalMarkingType = MarkingType;
    AppearanceData.MarkingColor = Color;
    UpdateMaterialParameters();
}

void AChar_MetaHumanBase::SetSkinTone(float ToneValue)
{
    AppearanceData.SkinTone = FMath::Clamp(ToneValue, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void AChar_MetaHumanBase::SetBodyBuild(float BuildValue)
{
    AppearanceData.BodyBuild = FMath::Clamp(BuildValue, 0.0f, 1.0f);
    
    // Apply body build to mesh morph targets if available
    if (GetMesh() && GetMesh()->GetSkeletalMeshAsset())
    {
        GetMesh()->SetMorphTarget(TEXT("BodyBuild"), BuildValue);
    }
}

void AChar_MetaHumanBase::EquipItem(const FString& SlotName, UStaticMesh* ItemMesh, const FTransform& RelativeTransform)
{
    if (!EquipmentSlots.Contains(SlotName))
    {
        return;
    }

    FChar_EquipmentSlot& Slot = EquipmentSlots[SlotName];
    Slot.EquippedMesh = ItemMesh;
    Slot.RelativeTransform = RelativeTransform;

    // Apply to appropriate component
    UStaticMeshComponent* TargetComponent = nullptr;
    
    if (SlotName == TEXT("MainWeapon"))
    {
        TargetComponent = MainWeaponComponent;
    }
    else if (SlotName == TEXT("SecondaryWeapon"))
    {
        TargetComponent = SecondaryWeaponComponent;
    }
    else if (SlotName == TEXT("Shield"))
    {
        TargetComponent = ShieldComponent;
    }
    else if (SlotName == TEXT("Accessory"))
    {
        TargetComponent = AccessoryComponent;
    }

    if (TargetComponent)
    {
        TargetComponent->SetStaticMesh(ItemMesh);
        TargetComponent->SetRelativeTransform(RelativeTransform);
        TargetComponent->SetVisibility(ItemMesh != nullptr);
    }
}

void AChar_MetaHumanBase::UnequipItem(const FString& SlotName)
{
    EquipItem(SlotName, nullptr, FTransform::Identity);
}

void AChar_MetaHumanBase::EquipMainWeapon(UStaticMesh* WeaponMesh)
{
    EquipItem(TEXT("MainWeapon"), WeaponMesh, FTransform::Identity);
}

void AChar_MetaHumanBase::EquipShield(UStaticMesh* ShieldMesh)
{
    EquipItem(TEXT("Shield"), ShieldMesh, FTransform::Identity);
}

void AChar_MetaHumanBase::EquipAccessory(UStaticMesh* AccessoryMesh)
{
    EquipItem(TEXT("Accessory"), AccessoryMesh, FTransform::Identity);
}

void AChar_MetaHumanBase::UpdateCharacterLighting(float FireIntensity, FLinearColor AmbientColor)
{
    ApplyLightingToMaterials(FireIntensity, AmbientColor);
}

void AChar_MetaHumanBase::UpdateMaterialParameters()
{
    if (!GetMesh())
    {
        return;
    }

    // Create dynamic material instances if needed
    if (!BodyMaterialInstance && GetMesh()->GetMaterial(0))
    {
        BodyMaterialInstance = GetMesh()->CreateDynamicMaterialInstance(0);
    }
    
    if (!ClothingMaterialInstance && GetMesh()->GetMaterial(1))
    {
        ClothingMaterialInstance = GetMesh()->CreateDynamicMaterialInstance(1);
    }

    // Apply appearance parameters
    if (BodyMaterialInstance)
    {
        BodyMaterialInstance->SetScalarParameterValue(TEXT("SkinTone"), AppearanceData.SkinTone);
        BodyMaterialInstance->SetScalarParameterValue(TEXT("BodyBuild"), AppearanceData.BodyBuild);
        BodyMaterialInstance->SetVectorParameterValue(TEXT("TribalMarkingColor"), AppearanceData.MarkingColor);
        BodyMaterialInstance->SetScalarParameterValue(TEXT("TribalMarkingType"), AppearanceData.TribalMarkingType);
    }

    if (ClothingMaterialInstance)
    {
        ClothingMaterialInstance->SetScalarParameterValue(TEXT("ClothingStyle"), AppearanceData.ClothingStyle);
    }
}

void AChar_MetaHumanBase::ApplyLightingToMaterials(float FireIntensity, FLinearColor AmbientColor)
{
    if (BodyMaterialInstance)
    {
        BodyMaterialInstance->SetScalarParameterValue(TEXT("FireLightIntensity"), FireIntensity);
        BodyMaterialInstance->SetVectorParameterValue(TEXT("AmbientLightColor"), AmbientColor);
    }

    if (ClothingMaterialInstance)
    {
        ClothingMaterialInstance->SetScalarParameterValue(TEXT("FireLightIntensity"), FireIntensity);
        ClothingMaterialInstance->SetVectorParameterValue(TEXT("AmbientLightColor"), AmbientColor);
    }

    // Apply to equipment materials
    if (MainWeaponComponent && MainWeaponComponent->GetMaterial(0))
    {
        UMaterialInstanceDynamic* WeaponMaterial = MainWeaponComponent->CreateDynamicMaterialInstance(0);
        if (WeaponMaterial)
        {
            WeaponMaterial->SetScalarParameterValue(TEXT("FireLightIntensity"), FireIntensity);
            WeaponMaterial->SetVectorParameterValue(TEXT("AmbientLightColor"), AmbientColor);
        }
    }
}