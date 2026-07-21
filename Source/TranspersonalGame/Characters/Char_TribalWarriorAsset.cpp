#include "Char_TribalWarriorAsset.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    CharacterMesh = nullptr;
    BodyScale = 1.0f;
    BattleWearLevel = 0.0f;
    TribalRank = 1;

    InitializeDefaultMaterials();
    SetupDefaultEquipment();
}

void UChar_TribalWarriorAsset::InitializeDefaultMaterials()
{
    // Create default material variant
    FChar_MaterialVariant DefaultVariant;
    DefaultVariant.VariantName = TEXT("Default Tribal");
    // Materials will be set via Blueprint or loaded from content
    MaterialVariants.Add(DefaultVariant);

    // Add weathered variant
    FChar_MaterialVariant WeatheredVariant;
    WeatheredVariant.VariantName = TEXT("Battle Worn");
    MaterialVariants.Add(WeatheredVariant);

    // Add elder variant
    FChar_MaterialVariant ElderVariant;
    ElderVariant.VariantName = TEXT("Tribal Elder");
    MaterialVariants.Add(ElderVariant);
}

void UChar_TribalWarriorAsset::SetupDefaultEquipment()
{
    // Spear slot
    FChar_EquipmentSlot SpearSlot;
    SpearSlot.SlotName = TEXT("MainWeapon");
    SpearSlot.AttachSocketName = FName("hand_r_socket");
    EquipmentSlots.Add(SpearSlot);

    // Shield slot
    FChar_EquipmentSlot ShieldSlot;
    ShieldSlot.SlotName = TEXT("Shield");
    ShieldSlot.AttachSocketName = FName("hand_l_socket");
    EquipmentSlots.Add(ShieldSlot);

    // Necklace slot
    FChar_EquipmentSlot NecklaceSlot;
    NecklaceSlot.SlotName = TEXT("Necklace");
    NecklaceSlot.AttachSocketName = FName("neck_socket");
    EquipmentSlots.Add(NecklaceSlot);

    // Belt pouch slot
    FChar_EquipmentSlot PouchSlot;
    PouchSlot.SlotName = TEXT("BeltPouch");
    PouchSlot.AttachSocketName = FName("pelvis_socket");
    EquipmentSlots.Add(PouchSlot);
}

void UChar_TribalWarriorAsset::ApplyMaterialVariant(USkeletalMeshComponent* MeshComponent, int32 VariantIndex)
{
    if (!MeshComponent || !MaterialVariants.IsValidIndex(VariantIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid mesh component or material variant index"));
        return;
    }

    const FChar_MaterialVariant& Variant = MaterialVariants[VariantIndex];

    // Apply skin material
    if (Variant.SkinMaterial)
    {
        MeshComponent->SetMaterial(0, Variant.SkinMaterial);
    }

    // Apply clothing material
    if (Variant.ClothingMaterial)
    {
        MeshComponent->SetMaterial(1, Variant.ClothingMaterial);
    }

    // Apply hair material
    if (Variant.HairMaterial)
    {
        MeshComponent->SetMaterial(2, Variant.HairMaterial);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied material variant: %s"), *Variant.VariantName);
}

void UChar_TribalWarriorAsset::AttachEquipment(USkeletalMeshComponent* MeshComponent, const FString& SlotName)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid mesh component for equipment attachment"));
        return;
    }

    // Find the equipment slot
    FChar_EquipmentSlot* FoundSlot = EquipmentSlots.FindByPredicate([&SlotName](const FChar_EquipmentSlot& Slot)
    {
        return Slot.SlotName == SlotName;
    });

    if (!FoundSlot || !FoundSlot->EquipmentMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment slot not found or no mesh assigned: %s"), *SlotName);
        return;
    }

    // Create and attach static mesh component for equipment
    UStaticMeshComponent* EquipmentComponent = NewObject<UStaticMeshComponent>(MeshComponent->GetOwner());
    if (EquipmentComponent)
    {
        EquipmentComponent->SetStaticMesh(FoundSlot->EquipmentMesh);
        
        if (FoundSlot->EquipmentMaterial)
        {
            EquipmentComponent->SetMaterial(0, FoundSlot->EquipmentMaterial);
        }

        EquipmentComponent->AttachToComponent(MeshComponent, 
            FAttachmentTransformRules::SnapToTargetIncludingScale, 
            FoundSlot->AttachSocketName);

        UE_LOG(LogTemp, Log, TEXT("Attached equipment: %s to socket: %s"), 
            *SlotName, *FoundSlot->AttachSocketName.ToString());
    }
}

void UChar_TribalWarriorAsset::SetBattleWear(USkeletalMeshComponent* MeshComponent, float WearLevel)
{
    if (!MeshComponent)
    {
        return;
    }

    BattleWearLevel = FMath::Clamp(WearLevel, 0.0f, 1.0f);

    // Create dynamic material instances to modify wear parameters
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
    {
        UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComponent);
            if (DynamicMaterial)
            {
                // Set wear parameters (assuming materials have these parameters)
                DynamicMaterial->SetScalarParameterValue(FName("WearLevel"), BattleWearLevel);
                DynamicMaterial->SetScalarParameterValue(FName("DirtAmount"), BattleWearLevel * 0.8f);
                DynamicMaterial->SetScalarParameterValue(FName("ScarAmount"), BattleWearLevel * 0.6f);
                
                MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied battle wear level: %f"), BattleWearLevel);
}

bool UChar_TribalWarriorAsset::ValidateCharacterAsset() const
{
    bool bIsValid = true;

    // Check if character mesh is assigned
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Character mesh is not assigned"));
        bIsValid = false;
    }

    // Check if we have at least one material variant
    if (MaterialVariants.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No material variants defined"));
        bIsValid = false;
    }

    // Validate equipment slots
    for (const FChar_EquipmentSlot& Slot : EquipmentSlots)
    {
        if (Slot.SlotName.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Equipment slot has empty name"));
        }
        
        if (Slot.AttachSocketName == NAME_None)
        {
            UE_LOG(LogTemp, Warning, TEXT("Equipment slot %s has no attach socket"), *Slot.SlotName);
        }
    }

    // Validate scale range
    if (BodyScale < 0.8f || BodyScale > 1.2f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Body scale is outside recommended range: %f"), BodyScale);
    }

    UE_LOG(LogTemp, Log, TEXT("Character asset validation complete. Valid: %s"), bIsValid ? TEXT("Yes") : TEXT("No"));
    return bIsValid;
}