// Copyright Transpersonal Game Studio - ClothingSystem.cpp
// Modular Clothing System Implementation for Character Customization

#include "ClothingSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogClothingSystem, Log, All);

UClothingSystemComponent::UClothingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    OwnerCharacter = nullptr;
}

void UClothingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogClothingSystem, Error, TEXT("ClothingSystemComponent requires a Character owner"));
        return;
    }
    
    // Initialize with default clothing if available
    if (AvailableClothingSets.Num() > 0 && AvailableClothingSets[0])
    {
        EquipClothingSet(AvailableClothingSets[0]);
    }
    
    UE_LOG(LogClothingSystem, Log, TEXT("ClothingSystemComponent initialized for %s"), 
           *OwnerCharacter->GetName());
}

bool UClothingSystemComponent::EquipClothingPiece(const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogClothingSystem, Warning, TEXT("No owner character for clothing system"));
        return false;
    }
    
    // Add or replace clothing piece in current outfit
    CurrentOutfit.Add(ClothingPiece.Slot, ClothingPiece);
    
    // Update visual representation
    UpdateMeshComponent(ClothingPiece.Slot, ClothingPiece);
    UpdateMaterialParameters(ClothingPiece.Slot, ClothingPiece);
    
    UE_LOG(LogClothingSystem, Log, TEXT("Equipped %s to slot %d"), 
           *ClothingPiece.ClothingName, (int32)ClothingPiece.Slot);
    
    return true;
}

bool UClothingSystemComponent::UnequipClothingSlot(EClothingSlot Slot)
{
    if (!OwnerCharacter)
    {
        return false;
    }
    
    if (CurrentOutfit.Contains(Slot))
    {
        CurrentOutfit.Remove(Slot);
        
        // Hide or remove mesh component for this slot
        // This would typically involve hiding specific mesh sections
        // or swapping to a "naked" version for that slot
        
        UE_LOG(LogClothingSystem, Log, TEXT("Unequipped clothing from slot %d"), (int32)Slot);
        return true;
    }
    
    return false;
}

void UClothingSystemComponent::EquipClothingSet(const UClothingSet* ClothingSet)
{
    if (!ClothingSet || !OwnerCharacter)
    {
        UE_LOG(LogClothingSystem, Warning, TEXT("Invalid clothing set or no owner character"));
        return;
    }
    
    // Clear current outfit
    CurrentOutfit.Empty();
    
    // Equip all pieces from the set
    for (const FClothingPiece& Piece : ClothingSet->ClothingPieces)
    {
        EquipClothingPiece(Piece);
    }
    
    UE_LOG(LogClothingSystem, Log, TEXT("Equipped clothing set: %s"), *ClothingSet->SetName);
}

FClothingPiece UClothingSystemComponent::GetClothingInSlot(EClothingSlot Slot) const
{
    if (CurrentOutfit.Contains(Slot))
    {
        return CurrentOutfit[Slot];
    }
    
    // Return empty clothing piece if slot is empty
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
    
    // Update condition based on damage
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
    
    // Update visual representation
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogClothingSystem, Log, TEXT("Damaged %s - damage level now %.2f"), 
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
    
    // Update visual representation
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogClothingSystem, Log, TEXT("Made %s dirtier - dirt level now %.2f"), 
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
    
    // Update visual representation
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogClothingSystem, Log, TEXT("Cleaned %s - dirt level now %.2f"), 
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
    ClothingPiece.WearLevel = FMath::Clamp(ClothingPiece.WearLevel - (RepairAmount * 0.5f), 0.0f, 1.0f);
    
    // Update condition based on new damage level
    if (ClothingPiece.DamageLevel < 0.1f)
    {
        ClothingPiece.Condition = EClothingCondition::Good;
    }
    else if (ClothingPiece.DamageLevel < 0.3f)
    {
        ClothingPiece.Condition = EClothingCondition::Worn;
    }
    else if (ClothingPiece.DamageLevel < 0.6f)
    {
        ClothingPiece.Condition = EClothingCondition::Damaged;
    }
    
    // Update visual representation
    UpdateMaterialParameters(Slot, ClothingPiece);
    
    UE_LOG(LogClothingSystem, Log, TEXT("Repaired %s - damage level now %.2f"), 
           *ClothingPiece.ClothingName, ClothingPiece.DamageLevel);
}

float UClothingSystemComponent::GetTotalColdProtection() const
{
    float TotalProtection = 0.0f;
    int32 PieceCount = 0;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        const FClothingPiece& Piece = OutfitPair.Value;
        TotalProtection += Piece.ColdProtection;
        PieceCount++;
    }
    
    return PieceCount > 0 ? TotalProtection / PieceCount : 0.0f;
}

float UClothingSystemComponent::GetTotalHeatProtection() const
{
    float TotalProtection = 0.0f;
    int32 PieceCount = 0;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        const FClothingPiece& Piece = OutfitPair.Value;
        TotalProtection += Piece.HeatProtection;
        PieceCount++;
    }
    
    return PieceCount > 0 ? TotalProtection / PieceCount : 0.0f;
}

float UClothingSystemComponent::GetTotalWaterResistance() const
{
    float TotalResistance = 0.0f;
    int32 PieceCount = 0;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        const FClothingPiece& Piece = OutfitPair.Value;
        TotalResistance += Piece.WaterResistance;
        PieceCount++;
    }
    
    return PieceCount > 0 ? TotalResistance / PieceCount : 0.0f;
}

float UClothingSystemComponent::GetTotalPhysicalProtection() const
{
    float TotalProtection = 0.0f;
    int32 PieceCount = 0;
    
    for (const auto& OutfitPair : CurrentOutfit)
    {
        const FClothingPiece& Piece = OutfitPair.Value;
        TotalProtection += Piece.PhysicalProtection;
        PieceCount++;
    }
    
    return PieceCount > 0 ? TotalProtection / PieceCount : 0.0f;
}

void UClothingSystemComponent::UpdateClothingVisuals()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update all equipped clothing pieces
    for (const auto& OutfitPair : CurrentOutfit)
    {
        UpdateMeshComponent(OutfitPair.Key, OutfitPair.Value);
        UpdateMaterialParameters(OutfitPair.Key, OutfitPair.Value);
    }
    
    UE_LOG(LogClothingSystem, Log, TEXT("Updated clothing visuals for %s"), 
           *OwnerCharacter->GetName());
}

void UClothingSystemComponent::ApplyWeatherEffects(float RainIntensity, float Temperature)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Apply weather-based wear and dirt accumulation
    float WeatherDamage = 0.0f;
    float WeatherDirt = 0.0f;
    
    // Rain effects
    if (RainIntensity > 0.1f)
    {
        WeatherDirt += RainIntensity * 0.02f; // Rain makes things muddy
        
        // Check water resistance
        float WaterResistance = GetTotalWaterResistance();
        if (WaterResistance < 0.5f)
        {
            WeatherDamage += (RainIntensity * (1.0f - WaterResistance)) * 0.01f;
        }
    }
    
    // Temperature effects
    if (Temperature > 35.0f) // Hot weather
    {
        WeatherDirt += (Temperature - 35.0f) * 0.001f; // Sweat and dust
    }
    else if (Temperature < 0.0f) // Cold weather
    {
        WeatherDamage += FMath::Abs(Temperature) * 0.0005f; // Cold damage
    }
    
    // Apply effects to all clothing
    for (auto& OutfitPair : CurrentOutfit)
    {
        if (WeatherDamage > 0.0f)
        {
            DamageClothing(OutfitPair.Key, WeatherDamage);
        }
        if (WeatherDirt > 0.0f)
        {
            MakeClothingDirty(OutfitPair.Key, WeatherDirt);
        }
    }
}

void UClothingSystemComponent::UpdateMeshComponent(EClothingSlot Slot, const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Load the clothing mesh if specified
    if (ClothingPiece.ClothingMesh.IsValid())
    {
        // In a real implementation, this would involve:
        // 1. Loading the skeletal mesh asset
        // 2. Applying it to the appropriate mesh section or component
        // 3. Handling mesh merging or layering
        
        UE_LOG(LogClothingSystem, Log, TEXT("Updating mesh for slot %d with %s"), 
               (int32)Slot, *ClothingPiece.ClothingName);
    }
}

void UClothingSystemComponent::UpdateMaterialParameters(EClothingSlot Slot, const FClothingPiece& ClothingPiece)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Create or get dynamic material instance for this clothing piece
    for (int32 MaterialIndex = 0; MaterialIndex < ClothingPiece.Materials.Num(); MaterialIndex++)
    {
        if (ClothingPiece.Materials[MaterialIndex].IsValid())
        {
            UMaterialInstanceDynamic* DynamicMat = MeshComp->CreateDynamicMaterialInstance(
                MaterialIndex, ClothingPiece.Materials[MaterialIndex].LoadSynchronous());
            
            if (DynamicMat)
            {
                // Apply clothing condition parameters
                DynamicMat->SetScalarParameterValue(TEXT("DirtLevel"), ClothingPiece.DirtLevel);
                DynamicMat->SetScalarParameterValue(TEXT("WearLevel"), ClothingPiece.WearLevel);
                DynamicMat->SetScalarParameterValue(TEXT("DamageLevel"), ClothingPiece.DamageLevel);
                
                // Apply colors
                DynamicMat->SetVectorParameterValue(TEXT("PrimaryColor"), ClothingPiece.PrimaryColor);
                DynamicMat->SetVectorParameterValue(TEXT("SecondaryColor"), ClothingPiece.SecondaryColor);
                
                UE_LOG(LogClothingSystem, Log, TEXT("Updated material parameters for %s"), 
                       *ClothingPiece.ClothingName);
            }
        }
    }
}