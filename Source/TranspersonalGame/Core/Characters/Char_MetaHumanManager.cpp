#include "Char_MetaHumanManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

AChar_MetaHumanManager::AChar_MetaHumanManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create character mesh component
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    CharacterMesh->SetupAttachment(RootComponent);
    CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

    // Initialize default appearance config
    AppearanceConfig = FChar_AppearanceConfig();

    // Initialize equipment slots
    InitializeEquipmentSlots();
}

void AChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();

    // Create dynamic materials for customization
    CreateDynamicMaterials();

    // Apply initial appearance configuration
    ApplyAppearanceConfig(AppearanceConfig);

    // Setup equipment components
    SetupEquipmentComponents();
}

void AChar_MetaHumanManager::InitializeEquipmentSlots()
{
    EquipmentSlots.Empty();

    // Primary weapon slot (right hand)
    FChar_EquipmentSlot WeaponSlot;
    WeaponSlot.SlotName = FName("PrimaryWeapon");
    WeaponSlot.AttachmentSocket = FName("hand_r");
    WeaponSlot.RelativeTransform = FTransform(FRotator(0, 90, 0), FVector(0, 0, 0), FVector(1.0f));
    EquipmentSlots.Add(WeaponSlot);

    // Shield slot (left hand)
    FChar_EquipmentSlot ShieldSlot;
    ShieldSlot.SlotName = FName("Shield");
    ShieldSlot.AttachmentSocket = FName("hand_l");
    ShieldSlot.RelativeTransform = FTransform(FRotator(0, 0, 90), FVector(0, 0, 0), FVector(1.0f));
    EquipmentSlots.Add(ShieldSlot);

    // Back weapon slot
    FChar_EquipmentSlot BackWeaponSlot;
    BackWeaponSlot.SlotName = FName("BackWeapon");
    BackWeaponSlot.AttachmentSocket = FName("spine_03");
    BackWeaponSlot.RelativeTransform = FTransform(FRotator(0, 0, 180), FVector(-10, 0, 0), FVector(1.0f));
    EquipmentSlots.Add(BackWeaponSlot);

    // Hip pouch slot
    FChar_EquipmentSlot PouchSlot;
    PouchSlot.SlotName = FName("HipPouch");
    PouchSlot.AttachmentSocket = FName("pelvis");
    PouchSlot.RelativeTransform = FTransform(FRotator(0, 45, 0), FVector(0, 15, -5), FVector(0.8f));
    EquipmentSlots.Add(PouchSlot);

    // Necklace slot
    FChar_EquipmentSlot NecklaceSlot;
    NecklaceSlot.SlotName = FName("Necklace");
    NecklaceSlot.AttachmentSocket = FName("neck_01");
    NecklaceSlot.RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1.0f));
    EquipmentSlots.Add(NecklaceSlot);

    // Arm band slot
    FChar_EquipmentSlot ArmBandSlot;
    ArmBandSlot.SlotName = FName("ArmBand");
    ArmBandSlot.AttachmentSocket = FName("upperarm_l");
    ArmBandSlot.RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1.0f));
    EquipmentSlots.Add(ArmBandSlot);
}

void AChar_MetaHumanManager::SetupEquipmentComponents()
{
    // Clear existing equipment components
    for (UStaticMeshComponent* Component : EquipmentComponents)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }
    EquipmentComponents.Empty();

    // Create equipment components for each slot
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        FString ComponentName = FString::Printf(TEXT("Equipment_%s"), *EquipmentSlots[i].SlotName.ToString());
        UStaticMeshComponent* EquipmentComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
        
        if (EquipmentComponent && CharacterMesh)
        {
            EquipmentComponent->SetupAttachment(CharacterMesh, EquipmentSlots[i].AttachmentSocket);
            EquipmentComponent->SetRelativeTransform(EquipmentSlots[i].RelativeTransform);
            EquipmentComponent->SetVisibility(false); // Hidden by default
            EquipmentComponents.Add(EquipmentComponent);
        }
    }
}

void AChar_MetaHumanManager::ApplyAppearanceConfig(const FChar_AppearanceConfig& NewConfig)
{
    AppearanceConfig = NewConfig;

    // Apply body scale
    if (CharacterMesh)
    {
        CharacterMesh->SetRelativeScale3D(FVector(AppearanceConfig.BodyScale));
    }

    // Update material parameters
    UpdateMaterialParameters();

    // Log appearance change for debugging
    UE_LOG(LogTemp, Log, TEXT("Applied appearance config: SkinTone=%d, BodyBuild=%d, Markings=%d"), 
           (int32)AppearanceConfig.SkinTone, (int32)AppearanceConfig.BodyBuild, (int32)AppearanceConfig.TribalMarkings);
}

void AChar_MetaHumanManager::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    AppearanceConfig.SkinTone = NewSkinTone;
    UpdateMaterialParameters();
}

void AChar_MetaHumanManager::SetBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    AppearanceConfig.BodyBuild = NewBodyBuild;
    
    // Adjust body scale based on build
    float ScaleMultiplier = 1.0f;
    switch (NewBodyBuild)
    {
        case EChar_BodyBuild::Lean:
            ScaleMultiplier = 0.95f;
            break;
        case EChar_BodyBuild::Average:
            ScaleMultiplier = 1.0f;
            break;
        case EChar_BodyBuild::Muscular:
            ScaleMultiplier = 1.05f;
            break;
        case EChar_BodyBuild::Heavy:
            ScaleMultiplier = 1.1f;
            break;
    }
    
    AppearanceConfig.BodyScale = ScaleMultiplier;
    if (CharacterMesh)
    {
        CharacterMesh->SetRelativeScale3D(FVector(ScaleMultiplier));
    }
}

void AChar_MetaHumanManager::SetTribalMarkings(EChar_TribalMarkings NewMarkings, FLinearColor MarkingColor)
{
    AppearanceConfig.TribalMarkings = NewMarkings;
    AppearanceConfig.MarkingColor = MarkingColor;
    UpdateMaterialParameters();
}

void AChar_MetaHumanManager::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    AppearanceConfig.ClothingStyle = NewStyle;
    UpdateMaterialParameters();
}

bool AChar_MetaHumanManager::EquipItem(FName SlotName, UStaticMesh* ItemMesh)
{
    if (!ItemMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot equip null mesh to slot %s"), *SlotName.ToString());
        return false;
    }

    // Find the equipment slot
    int32 SlotIndex = -1;
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        if (EquipmentSlots[i].SlotName == SlotName)
        {
            SlotIndex = i;
            break;
        }
    }

    if (SlotIndex == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment slot %s not found"), *SlotName.ToString());
        return false;
    }

    // Update slot data
    EquipmentSlots[SlotIndex].EquippedMesh = ItemMesh;
    EquipmentSlots[SlotIndex].bIsEquipped = true;

    // Update equipment component
    if (SlotIndex < EquipmentComponents.Num() && EquipmentComponents[SlotIndex])
    {
        EquipmentComponents[SlotIndex]->SetStaticMesh(ItemMesh);
        EquipmentComponents[SlotIndex]->SetVisibility(true);
        EquipmentComponents[SlotIndex]->SetRelativeTransform(EquipmentSlots[SlotIndex].RelativeTransform);
    }

    UE_LOG(LogTemp, Log, TEXT("Equipped %s to slot %s"), *ItemMesh->GetName(), *SlotName.ToString());
    return true;
}

bool AChar_MetaHumanManager::UnequipItem(FName SlotName)
{
    // Find the equipment slot
    int32 SlotIndex = -1;
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        if (EquipmentSlots[i].SlotName == SlotName)
        {
            SlotIndex = i;
            break;
        }
    }

    if (SlotIndex == -1)
    {
        return false;
    }

    // Update slot data
    EquipmentSlots[SlotIndex].EquippedMesh = nullptr;
    EquipmentSlots[SlotIndex].bIsEquipped = false;

    // Hide equipment component
    if (SlotIndex < EquipmentComponents.Num() && EquipmentComponents[SlotIndex])
    {
        EquipmentComponents[SlotIndex]->SetStaticMesh(nullptr);
        EquipmentComponents[SlotIndex]->SetVisibility(false);
    }

    UE_LOG(LogTemp, Log, TEXT("Unequipped item from slot %s"), *SlotName.ToString());
    return true;
}

void AChar_MetaHumanManager::UnequipAllItems()
{
    for (const FChar_EquipmentSlot& Slot : EquipmentSlots)
    {
        UnequipItem(Slot.SlotName);
    }
}

UStaticMeshComponent* AChar_MetaHumanManager::GetEquipmentComponent(FName SlotName)
{
    int32 SlotIndex = -1;
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        if (EquipmentSlots[i].SlotName == SlotName)
        {
            SlotIndex = i;
            break;
        }
    }

    if (SlotIndex >= 0 && SlotIndex < EquipmentComponents.Num())
    {
        return EquipmentComponents[SlotIndex];
    }

    return nullptr;
}

void AChar_MetaHumanManager::LoadMetaHumanPreset(int32 PresetIndex)
{
    if (PresetIndex >= 0 && PresetIndex < MetaHumanPresets.Num() && CharacterMesh)
    {
        USkeletalMesh* PresetMesh = MetaHumanPresets[PresetIndex].LoadSynchronous();
        if (PresetMesh)
        {
            CharacterMesh->SetSkeletalMesh(PresetMesh);
            CreateDynamicMaterials();
            UpdateMaterialParameters();
            UE_LOG(LogTemp, Log, TEXT("Loaded MetaHuman preset %d"), PresetIndex);
        }
    }
}

void AChar_MetaHumanManager::RandomizeAppearance()
{
    // Randomize appearance parameters
    AppearanceConfig.SkinTone = static_cast<EChar_SkinTone>(FMath::RandRange(0, 4));
    AppearanceConfig.BodyBuild = static_cast<EChar_BodyBuild>(FMath::RandRange(0, 3));
    AppearanceConfig.TribalMarkings = static_cast<EChar_TribalMarkings>(FMath::RandRange(0, 4));
    AppearanceConfig.ClothingStyle = static_cast<EChar_ClothingStyle>(FMath::RandRange(0, 3));
    
    // Random marking color
    AppearanceConfig.MarkingColor = FLinearColor(
        FMath::RandRange(0.1f, 1.0f),
        FMath::RandRange(0.1f, 1.0f),
        FMath::RandRange(0.1f, 1.0f),
        1.0f
    );
    
    // Random body scale variation
    AppearanceConfig.BodyScale = FMath::RandRange(0.9f, 1.1f);

    ApplyAppearanceConfig(AppearanceConfig);
    UE_LOG(LogTemp, Log, TEXT("Randomized character appearance"));
}

void AChar_MetaHumanManager::CreateTribalWarriorVariation(int32 VariationSeed)
{
    // Use seed for consistent randomization
    FMath::RandInit(VariationSeed);

    // Create warrior-specific appearance
    AppearanceConfig.BodyBuild = static_cast<EChar_BodyBuild>(FMath::RandRange(1, 2)); // Average to Muscular
    AppearanceConfig.TribalMarkings = static_cast<EChar_TribalMarkings>(FMath::RandRange(1, 2)); // Warrior or Hunter
    AppearanceConfig.ClothingStyle = static_cast<EChar_ClothingStyle>(FMath::RandRange(1, 3)); // Basic to Ceremonial
    
    // Warrior colors tend to be earth tones and red
    float ColorVariation = FMath::RandRange(0.0f, 1.0f);
    if (ColorVariation < 0.4f)
    {
        AppearanceConfig.MarkingColor = FLinearColor(0.8f, 0.2f, 0.1f, 1.0f); // Red
    }
    else if (ColorVariation < 0.7f)
    {
        AppearanceConfig.MarkingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Brown
    }
    else
    {
        AppearanceConfig.MarkingColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); // Black
    }

    ApplyAppearanceConfig(AppearanceConfig);
    UE_LOG(LogTemp, Log, TEXT("Created tribal warrior variation with seed %d"), VariationSeed);
}

void AChar_MetaHumanManager::RefreshCharacterAppearance()
{
    CreateDynamicMaterials();
    UpdateMaterialParameters();
    UE_LOG(LogTemp, Log, TEXT("Refreshed character appearance"));
}

void AChar_MetaHumanManager::CreateDynamicMaterials()
{
    if (!CharacterMesh)
    {
        return;
    }

    DynamicMaterials.Empty();

    // Create dynamic material instances for each material slot
    int32 MaterialCount = CharacterMesh->GetNumMaterials();
    for (int32 i = 0; i < MaterialCount; i++)
    {
        UMaterialInterface* BaseMaterial = CharacterMesh->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                CharacterMesh->SetMaterial(i, DynamicMaterial);
                DynamicMaterials.Add(DynamicMaterial);
            }
        }
    }
}

void AChar_MetaHumanManager::UpdateMaterialParameters()
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material)
        {
            ApplySkinToneToMaterial(Material, AppearanceConfig.SkinTone);
            ApplyTribalMarkingsToMaterial(Material, AppearanceConfig.TribalMarkings, AppearanceConfig.MarkingColor);
        }
    }
}

UMaterialInstanceDynamic* AChar_MetaHumanManager::GetOrCreateDynamicMaterial(int32 MaterialIndex)
{
    if (MaterialIndex >= 0 && MaterialIndex < DynamicMaterials.Num())
    {
        return DynamicMaterials[MaterialIndex];
    }
    return nullptr;
}

void AChar_MetaHumanManager::ApplySkinToneToMaterial(UMaterialInstanceDynamic* Material, EChar_SkinTone SkinTone)
{
    if (!Material)
    {
        return;
    }

    FLinearColor SkinColor;
    switch (SkinTone)
    {
        case EChar_SkinTone::VeryLight:
            SkinColor = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
            break;
        case EChar_SkinTone::Light:
            SkinColor = FLinearColor(0.85f, 0.75f, 0.65f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            SkinColor = FLinearColor(0.75f, 0.65f, 0.55f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            SkinColor = FLinearColor(0.55f, 0.45f, 0.35f, 1.0f);
            break;
        case EChar_SkinTone::VeryDark:
            SkinColor = FLinearColor(0.35f, 0.25f, 0.20f, 1.0f);
            break;
        default:
            SkinColor = FLinearColor(0.75f, 0.65f, 0.55f, 1.0f);
            break;
    }

    Material->SetVectorParameterValue(FName("SkinTone"), SkinColor);
}

void AChar_MetaHumanManager::ApplyTribalMarkingsToMaterial(UMaterialInstanceDynamic* Material, EChar_TribalMarkings Markings, FLinearColor Color)
{
    if (!Material)
    {
        return;
    }

    float MarkingIntensity = 0.0f;
    float MarkingPattern = 0.0f;

    switch (Markings)
    {
        case EChar_TribalMarkings::None:
            MarkingIntensity = 0.0f;
            break;
        case EChar_TribalMarkings::Warrior:
            MarkingIntensity = 0.8f;
            MarkingPattern = 1.0f;
            break;
        case EChar_TribalMarkings::Hunter:
            MarkingIntensity = 0.6f;
            MarkingPattern = 2.0f;
            break;
        case EChar_TribalMarkings::Shaman:
            MarkingIntensity = 1.0f;
            MarkingPattern = 3.0f;
            break;
        case EChar_TribalMarkings::Elder:
            MarkingIntensity = 0.9f;
            MarkingPattern = 4.0f;
            break;
    }

    Material->SetVectorParameterValue(FName("TribalMarkingColor"), Color);
    Material->SetScalarParameterValue(FName("TribalMarkingIntensity"), MarkingIntensity);
    Material->SetScalarParameterValue(FName("TribalMarkingPattern"), MarkingPattern);
}