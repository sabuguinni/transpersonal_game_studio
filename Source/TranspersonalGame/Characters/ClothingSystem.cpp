#include "ClothingSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UClothingSystemComponent::UClothingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UClothingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClothingSystemComponent: Owner is not a Character!"));
        return;
    }
    
    // Initialize with default clothing if available
    if (AvailableClothingSets.Num() > 0 && AvailableClothingSets[0])
    {
        EquipClothingSet(AvailableClothingSets[0]);
    }
}

bool UClothingSystemComponent::EquipClothingPiece(const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter)
    {
        return false;
    }
    
    // Store the clothing piece in the current outfit
    CurrentOutfit.Add(ClothingPiece.Slot, ClothingPiece);
    
    // Update the visual representation
    UpdateMeshComponent(ClothingPiece.Slot, ClothingPiece);
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Equipped %s in slot %d"), 
           *ClothingPiece.ClothingName, (int32)ClothingPiece.Slot);
    
    return true;
}

bool UClothingSystemComponent::UnequipClothingSlot(EClothingSlot Slot)
{
    if (!CurrentOutfit.Contains(Slot))
    {
        return false;
    }
    
    CurrentOutfit.Remove(Slot);
    
    // Hide or remove the mesh component for this slot
    // Implementation would depend on how clothing meshes are structured
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Unequipped clothing from slot %d"), (int32)Slot);
    
    return true;
}

void UClothingSystemComponent::EquipClothingSet(const UClothingSet* ClothingSet)
{
    if (!ClothingSet || !OwnerCharacter)
    {
        return;
    }
    
    // Clear current outfit
    CurrentOutfit.Empty();
    
    // Equip all pieces from the set
    for (const FClothingPiece& Piece : ClothingSet->ClothingPieces)
    {
        EquipClothingPiece(Piece);
    }
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Equipped clothing set: %s"), *ClothingSet->SetName);
}

FClothingPiece UClothingSystemComponent::GetClothingInSlot(EClothingSlot Slot) const
{
    if (CurrentOutfit.Contains(Slot))
    {
        return CurrentOutfit[Slot];
    }
    
    return FClothingPiece();
}

void UClothingSystemComponent::DamageClothing(EClothingSlot Slot, float DamageAmount)
{
    if (!CurrentOutfit.Contains(Slot))
    {
        return;
    }
    
    FClothingPiece& ClothingPiece = CurrentOutfit[Slot];
    ClothingPiece.DamageLevel = FMath::Clamp(ClothingPiece.DamageLevel + DamageAmount, 0.0f, 1.0f);
    
    // Update condition based on damage level
    if (ClothingPiece.DamageLevel > 0.8f)
    {
        ClothingPiece.Condition = EClothingCondition::Torn;
    }
    else if (ClothingPiece.DamageLevel > 0.6f)
    {
        ClothingPiece.Condition = EClothingCondition::Damaged;
    }
    else if (ClothingPiece.DamageLevel > 0.3f)
    {
        ClothingPiece.Condition = EClothingCondition::Worn;
    }
    
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Damaged %s, damage level now: %f"), 
           *ClothingPiece.ClothingName, ClothingPiece.DamageLevel);
}

void UClothingSystemComponent::MakeClothingDirty(EClothingSlot Slot, float DirtAmount)
{
    if (!CurrentOutfit.Contains(Slot))
    {
        return;
    }
    
    FClothingPiece& ClothingPiece = CurrentOutfit[Slot];
    ClothingPiece.DirtLevel = FMath::Clamp(ClothingPiece.DirtLevel + DirtAmount, 0.0f, 1.0f);
    
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Made %s dirty, dirt level now: %f"), 
           *ClothingPiece.ClothingName, ClothingPiece.DirtLevel);
}

void UClothingSystemComponent::CleanClothing(EClothingSlot Slot, float CleanAmount)
{
    if (!CurrentOutfit.Contains(Slot))
    {
        return;
    }
    
    FClothingPiece& ClothingPiece = CurrentOutfit[Slot];
    ClothingPiece.DirtLevel = FMath::Clamp(ClothingPiece.DirtLevel - CleanAmount, 0.0f, 1.0f);
    
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Cleaned %s, dirt level now: %f"), 
           *ClothingPiece.ClothingName, ClothingPiece.DirtLevel);
}

void UClothingSystemComponent::RepairClothing(EClothingSlot Slot, float RepairAmount)
{
    if (!CurrentOutfit.Contains(Slot))
    {
        return;
    }
    
    FClothingPiece& ClothingPiece = CurrentOutfit[Slot];
    ClothingPiece.DamageLevel = FMath::Clamp(ClothingPiece.DamageLevel - RepairAmount, 0.0f, 1.0f);
    ClothingPiece.WearLevel = FMath::Clamp(ClothingPiece.WearLevel - RepairAmount * 0.5f, 0.0f, 1.0f);
    
    // Update condition based on new damage level
    if (ClothingPiece.DamageLevel < 0.1f)
    {
        ClothingPiece.Condition = EClothingCondition::Perfect;
    }
    else if (ClothingPiece.DamageLevel < 0.3f)
    {
        ClothingPiece.Condition = EClothingCondition::Good;
    }
    else if (ClothingPiece.DamageLevel < 0.6f)
    {
        ClothingPiece.Condition = EClothingCondition::Worn;
    }
    
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Repaired %s, damage level now: %f"), 
           *ClothingPiece.ClothingName, ClothingPiece.DamageLevel);
}

float UClothingSystemComponent::GetTotalColdProtection() const
{
    float TotalProtection = 0.0f;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        TotalProtection += OutfitPair.Value.ColdProtection;
    }
    
    return FMath::Clamp(TotalProtection, 0.0f, 1.0f);
}

float UClothingSystemComponent::GetTotalHeatProtection() const
{
    float TotalProtection = 0.0f;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        TotalProtection += OutfitPair.Value.HeatProtection;
    }
    
    return FMath::Clamp(TotalProtection, 0.0f, 1.0f);
}

float UClothingSystemComponent::GetTotalWaterResistance() const
{
    float TotalResistance = 0.0f;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        TotalResistance += OutfitPair.Value.WaterResistance;
    }
    
    return FMath::Clamp(TotalResistance, 0.0f, 1.0f);
}

float UClothingSystemComponent::GetTotalPhysicalProtection() const
{
    float TotalProtection = 0.0f;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        TotalProtection += OutfitPair.Value.PhysicalProtection;
    }
    
    return FMath::Clamp(TotalProtection, 0.0f, 1.0f);
}

void UClothingSystemComponent::UpdateClothingVisuals()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        UpdateMeshComponent(OutfitPair.Key, OutfitPair.Value);
        UpdateMaterialParameters(OutfitPair.Key, OutfitPair.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Updated all clothing visuals"));
}

void UClothingSystemComponent::ApplyWeatherEffects(float RainIntensity, float Temperature)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Apply dirt and wear based on weather conditions
    for (auto& OutfitPair : CurrentOutfit)
    {
        FClothingPiece& ClothingPiece = OutfitPair.Value;
        
        // Rain makes clothes dirty and can damage them
        if (RainIntensity > 0.1f)
        {
            float DirtIncrease = RainIntensity * 0.01f; // Small incremental dirt
            ClothingPiece.DirtLevel = FMath::Clamp(ClothingPiece.DirtLevel + DirtIncrease, 0.0f, 1.0f);
            
            // If clothing has low water resistance, it gets more damaged
            if (ClothingPiece.WaterResistance < 0.3f)
            {
                float DamageIncrease = RainIntensity * 0.005f;
                ClothingPiece.DamageLevel = FMath::Clamp(ClothingPiece.DamageLevel + DamageIncrease, 0.0f, 1.0f);
            }
        }
        
        // Extreme temperatures can damage clothing
        if (Temperature < -10.0f || Temperature > 40.0f)
        {
            float DamageIncrease = FMath::Abs(Temperature) * 0.001f;
            ClothingPiece.WearLevel = FMath::Clamp(ClothingPiece.WearLevel + DamageIncrease, 0.0f, 1.0f);
        }
        
        UpdateMaterialParameters(OutfitPair.Key, ClothingPiece);
    }
}

void UClothingSystemComponent::UpdateMeshComponent(EClothingSlot Slot, const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter || !ClothingPiece.ClothingMesh.IsValid())
    {
        return;
    }
    
    // This is a simplified implementation
    // In a real implementation, you would need to:
    // 1. Find or create a skeletal mesh component for this clothing slot
    // 2. Set the skeletal mesh asset
    // 3. Attach it to the character's skeleton
    // 4. Set up proper bone mapping and master pose component
    
    USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
    if (!CharacterMesh)
    {
        return;
    }
    
    // For now, just log that we would update the mesh
    UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Would update mesh for slot %d with %s"), 
           (int32)Slot, *ClothingPiece.ClothingName);
}

void UClothingSystemComponent::UpdateMaterialParameters(EClothingSlot Slot, const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
    if (!CharacterMesh)
    {
        return;
    }
    
    // Create or get dynamic material instance for this clothing piece
    // This is a simplified implementation
    for (int32 MaterialIndex = 0; MaterialIndex < ClothingPiece.Materials.Num(); MaterialIndex++)
    {
        if (ClothingPiece.Materials[MaterialIndex].IsValid())
        {
            UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
            if (DynamicMaterial)
            {
                // Set material parameters based on clothing condition
                DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), ClothingPiece.DirtLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), ClothingPiece.WearLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("DamageLevel"), ClothingPiece.DamageLevel);
                DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), ClothingPiece.PrimaryColor);
                DynamicMaterial->SetVectorParameterValue(TEXT("SecondaryColor"), ClothingPiece.SecondaryColor);
                
                UE_LOG(LogTemp, Log, TEXT("ClothingSystem: Updated material parameters for %s"), 
                       *ClothingPiece.ClothingName);
            }
        }
    }
}