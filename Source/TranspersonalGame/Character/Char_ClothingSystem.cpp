#include "Char_ClothingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UChar_ClothingSystem::UChar_ClothingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    TotalWarmth = 0.0f;
    TotalProtection = 0.0f;
}

void UChar_ClothingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CreateClothingMeshComponents();
    InitializeDefaultClothing();
    CalculateClothingStats();
}

void UChar_ClothingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UChar_ClothingSystem::EquipClothing(const FChar_ClothingItem& ClothingItem)
{
    if (ClothingItem.ItemName.IsEmpty())
    {
        return false;
    }

    // Store the clothing item in the appropriate slot
    EquippedClothing.Add(ClothingItem.EquipmentSlot, ClothingItem);
    
    // Update visual representation
    UpdateMeshComponent(ClothingItem.EquipmentSlot, ClothingItem);
    
    // Recalculate stats
    CalculateClothingStats();
    
    UE_LOG(LogTemp, Log, TEXT("Equipped %s in slot %d"), *ClothingItem.ItemName, (int32)ClothingItem.EquipmentSlot);
    
    return true;
}

bool UChar_ClothingSystem::UnequipClothing(EChar_EquipmentSlot Slot)
{
    if (EquippedClothing.Contains(Slot))
    {
        FChar_ClothingItem RemovedItem = EquippedClothing[Slot];
        EquippedClothing.Remove(Slot);
        
        // Hide the mesh component for this slot
        if (ClothingMeshComponents.Contains(Slot))
        {
            ClothingMeshComponents[Slot]->SetVisibility(false);
        }
        
        // Recalculate stats
        CalculateClothingStats();
        
        UE_LOG(LogTemp, Log, TEXT("Unequipped %s from slot %d"), *RemovedItem.ItemName, (int32)Slot);
        
        return true;
    }
    
    return false;
}

FChar_ClothingItem UChar_ClothingSystem::GetEquippedClothing(EChar_EquipmentSlot Slot) const
{
    if (EquippedClothing.Contains(Slot))
    {
        return EquippedClothing[Slot];
    }
    
    return FChar_ClothingItem();
}

void UChar_ClothingSystem::UpdateClothingVisuals()
{
    for (const auto& ClothingPair : EquippedClothing)
    {
        UpdateMeshComponent(ClothingPair.Key, ClothingPair.Value);
    }
}

void UChar_ClothingSystem::InitializeDefaultClothing()
{
    // Create basic animal hide tunic
    FChar_ClothingItem BasicTunic;
    BasicTunic.ItemName = TEXT("Animal Hide Tunic");
    BasicTunic.ClothingType = EChar_ClothingType::AnimalHide;
    BasicTunic.EquipmentSlot = EChar_EquipmentSlot::Torso;
    BasicTunic.WarmthValue = 15.0f;
    BasicTunic.ProtectionValue = 8.0f;
    BasicTunic.DurabilityValue = 80.0f;
    
    // Create basic leg wraps
    FChar_ClothingItem LegWraps;
    LegWraps.ItemName = TEXT("Leather Leg Wraps");
    LegWraps.ClothingType = EChar_ClothingType::LeatherGear;
    LegWraps.EquipmentSlot = EChar_EquipmentSlot::Legs;
    LegWraps.WarmthValue = 8.0f;
    LegWraps.ProtectionValue = 5.0f;
    LegWraps.DurabilityValue = 60.0f;
    
    // Create basic footwear
    FChar_ClothingItem BasicFootwear;
    BasicFootwear.ItemName = TEXT("Hide Foot Wraps");
    BasicFootwear.ClothingType = EChar_ClothingType::AnimalHide;
    BasicFootwear.EquipmentSlot = EChar_EquipmentSlot::Feet;
    BasicFootwear.WarmthValue = 5.0f;
    BasicFootwear.ProtectionValue = 3.0f;
    BasicFootwear.DurabilityValue = 40.0f;
    
    // Equip default clothing
    EquipClothing(BasicTunic);
    EquipClothing(LegWraps);
    EquipClothing(BasicFootwear);
    
    // Add to available clothing list
    AvailableClothing.Add(BasicTunic);
    AvailableClothing.Add(LegWraps);
    AvailableClothing.Add(BasicFootwear);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized default prehistoric clothing"));
}

void UChar_ClothingSystem::CreateClothingMeshComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Create mesh components for each equipment slot
    TArray<EChar_EquipmentSlot> AllSlots = {
        EChar_EquipmentSlot::Head,
        EChar_EquipmentSlot::Torso,
        EChar_EquipmentSlot::Legs,
        EChar_EquipmentSlot::Feet,
        EChar_EquipmentSlot::Hands,
        EChar_EquipmentSlot::Back,
        EChar_EquipmentSlot::Belt
    };
    
    for (EChar_EquipmentSlot Slot : AllSlots)
    {
        FString ComponentName = FString::Printf(TEXT("ClothingMesh_%d"), (int32)Slot);
        UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
        
        if (MeshComp)
        {
            MeshComp->SetupAttachment(Owner->GetRootComponent());
            MeshComp->SetVisibility(false); // Hidden by default
            ClothingMeshComponents.Add(Slot, MeshComp);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created clothing mesh components"));
}

void UChar_ClothingSystem::CalculateClothingStats()
{
    TotalWarmth = 0.0f;
    TotalProtection = 0.0f;
    
    for (const auto& ClothingPair : EquippedClothing)
    {
        const FChar_ClothingItem& Item = ClothingPair.Value;
        TotalWarmth += Item.WarmthValue;
        TotalProtection += Item.ProtectionValue;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Clothing stats - Warmth: %.1f, Protection: %.1f"), TotalWarmth, TotalProtection);
}

void UChar_ClothingSystem::UpdateMeshComponent(EChar_EquipmentSlot Slot, const FChar_ClothingItem& Item)
{
    if (!ClothingMeshComponents.Contains(Slot))
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = ClothingMeshComponents[Slot];
    if (!MeshComp)
    {
        return;
    }
    
    // Load and set the clothing mesh if available
    if (!Item.ClothingMesh.IsNull())
    {
        UStaticMesh* LoadedMesh = Item.ClothingMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComp->SetStaticMesh(LoadedMesh);
            MeshComp->SetVisibility(true);
        }
    }
    
    // Load and set the clothing material if available
    if (!Item.ClothingMaterial.IsNull())
    {
        UMaterialInterface* LoadedMaterial = Item.ClothingMaterial.LoadSynchronous();
        if (LoadedMaterial)
        {
            MeshComp->SetMaterial(0, LoadedMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated mesh component for slot %d with item %s"), (int32)Slot, *Item.ItemName);
}