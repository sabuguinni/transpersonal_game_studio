#include "Char_TribalCharacterManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UChar_TribalCharacterManager::UChar_TribalCharacterManager()
{
    MaxCharactersPerBiome = 5;
    CharacterSpawnRadius = 15000.0f;
    
    InitializeDefaultVariants();
    InitializeBiomeAdaptations();
}

void UChar_TribalCharacterManager::InitializeDefaultVariants()
{
    TribalVariants.Empty();
    
    // Savana Hunter - Earth tones, experienced tracker
    TribalVariants.Add(CreateVariant(TEXT("Savana_Hunter"), FLinearColor(0.8f, 0.4f, 0.2f, 1.0f), 75.0f));
    
    // Swamp Gatherer - Green tones, plant knowledge
    TribalVariants.Add(CreateVariant(TEXT("Swamp_Gatherer"), FLinearColor(0.2f, 0.6f, 0.3f, 1.0f), 60.0f));
    
    // Forest Scout - Brown/green camouflage, agile
    TribalVariants.Add(CreateVariant(TEXT("Forest_Scout"), FLinearColor(0.4f, 0.6f, 0.2f, 1.0f), 80.0f));
    
    // Desert Nomad - Light colors, heat resistance
    TribalVariants.Add(CreateVariant(TEXT("Desert_Nomad"), FLinearColor(0.9f, 0.8f, 0.6f, 1.0f), 70.0f));
    
    // Mountain Climber - Dark colors, cold resistance
    TribalVariants.Add(CreateVariant(TEXT("Mountain_Climber"), FLinearColor(0.3f, 0.3f, 0.5f, 1.0f), 85.0f));
}

void UChar_TribalCharacterManager::InitializeBiomeAdaptations()
{
    BiomeAdaptations.Empty();
    
    // Savana adaptation
    FChar_BiomeAdaptation SavanaAdapt;
    SavanaAdapt.BiomeType = EBiomeType::Savanna;
    SavanaAdapt.TemperatureResistance = 0.8f;
    SavanaAdapt.HumidityTolerance = 0.3f;
    SavanaAdapt.MovementSpeedModifier = 1.1f;
    SavanaAdapt.PreferredEquipment.Add(TEXT("Spear"));
    SavanaAdapt.PreferredEquipment.Add(TEXT("Leather_Armor"));
    BiomeAdaptations.Add(SavanaAdapt);
    
    // Swamp adaptation
    FChar_BiomeAdaptation SwampAdapt;
    SwampAdapt.BiomeType = EBiomeType::Swamp;
    SwampAdapt.TemperatureResistance = 0.6f;
    SwampAdapt.HumidityTolerance = 0.9f;
    SwampAdapt.MovementSpeedModifier = 0.9f;
    SwampAdapt.PreferredEquipment.Add(TEXT("Blowgun"));
    SwampAdapt.PreferredEquipment.Add(TEXT("Mud_Camouflage"));
    BiomeAdaptations.Add(SwampAdapt);
    
    // Forest adaptation
    FChar_BiomeAdaptation ForestAdapt;
    ForestAdapt.BiomeType = EBiomeType::Forest;
    ForestAdapt.TemperatureResistance = 0.7f;
    ForestAdapt.HumidityTolerance = 0.7f;
    ForestAdapt.MovementSpeedModifier = 1.2f;
    ForestAdapt.PreferredEquipment.Add(TEXT("Bow"));
    ForestAdapt.PreferredEquipment.Add(TEXT("Leaf_Cloak"));
    BiomeAdaptations.Add(ForestAdapt);
    
    // Desert adaptation
    FChar_BiomeAdaptation DesertAdapt;
    DesertAdapt.BiomeType = EBiomeType::Desert;
    DesertAdapt.TemperatureResistance = 0.9f;
    DesertAdapt.HumidityTolerance = 0.2f;
    DesertAdapt.MovementSpeedModifier = 1.0f;
    DesertAdapt.PreferredEquipment.Add(TEXT("Sling"));
    DesertAdapt.PreferredEquipment.Add(TEXT("Sand_Wraps"));
    BiomeAdaptations.Add(DesertAdapt);
    
    // Mountain adaptation
    FChar_BiomeAdaptation MountainAdapt;
    MountainAdapt.BiomeType = EBiomeType::Mountain;
    MountainAdapt.TemperatureResistance = 0.3f;
    MountainAdapt.HumidityTolerance = 0.5f;
    MountainAdapt.MovementSpeedModifier = 0.8f;
    MountainAdapt.PreferredEquipment.Add(TEXT("Club"));
    MountainAdapt.PreferredEquipment.Add(TEXT("Fur_Cloak"));
    BiomeAdaptations.Add(MountainAdapt);
}

FChar_TribalVariant UChar_TribalCharacterManager::CreateVariant(const FString& Name, const FLinearColor& WarPaint, float Experience)
{
    FChar_TribalVariant Variant;
    Variant.VariantName = Name;
    Variant.WarPaintColor = WarPaint;
    Variant.SurvivalExperience = Experience;
    return Variant;
}

FChar_TribalVariant UChar_TribalCharacterManager::GetVariantForBiome(EBiomeType BiomeType)
{
    for (const FChar_TribalVariant& Variant : TribalVariants)
    {
        if (Variant.VariantName.Contains(TEXT("Savana")) && BiomeType == EBiomeType::Savanna)
            return Variant;
        if (Variant.VariantName.Contains(TEXT("Swamp")) && BiomeType == EBiomeType::Swamp)
            return Variant;
        if (Variant.VariantName.Contains(TEXT("Forest")) && BiomeType == EBiomeType::Forest)
            return Variant;
        if (Variant.VariantName.Contains(TEXT("Desert")) && BiomeType == EBiomeType::Desert)
            return Variant;
        if (Variant.VariantName.Contains(TEXT("Mountain")) && BiomeType == EBiomeType::Mountain)
            return Variant;
    }
    
    // Return default variant if no match found
    return TribalVariants.Num() > 0 ? TribalVariants[0] : FChar_TribalVariant();
}

void UChar_TribalCharacterManager::GenerateTribalCharacters(UWorld* World, const TArray<FVector>& BiomeLocations)
{
    if (!World || BiomeLocations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Invalid world or biome locations"));
        return;
    }
    
    int32 BiomeIndex = 0;
    for (const FVector& BiomeCenter : BiomeLocations)
    {
        EBiomeType CurrentBiome = static_cast<EBiomeType>(BiomeIndex % 5);
        FChar_TribalVariant Variant = GetVariantForBiome(CurrentBiome);
        
        for (int32 i = 0; i < MaxCharactersPerBiome; i++)
        {
            // Random position within spawn radius
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::RandRange(0.0f, CharacterSpawnRadius);
            
            FVector SpawnLocation = BiomeCenter + FVector(
                Distance * FMath::Cos(Angle),
                Distance * FMath::Sin(Angle),
                200.0f // Safe spawn height
            );
            
            ACharacter* NewCharacter = SpawnTribalCharacter(World, SpawnLocation, Variant);
            if (NewCharacter)
            {
                ApplyBiomeAdaptation(NewCharacter, CurrentBiome);
                UE_LOG(LogTemp, Log, TEXT("Spawned tribal character: %s at biome %d"), *Variant.VariantName, BiomeIndex);
            }
        }
        
        BiomeIndex++;
    }
}

ACharacter* UChar_TribalCharacterManager::SpawnTribalCharacter(UWorld* World, const FVector& Location, const FChar_TribalVariant& Variant)
{
    if (!World)
        return nullptr;
    
    // Spawn basic character
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ACharacter* NewCharacter = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (NewCharacter)
    {
        // Set character label for identification
        NewCharacter->SetActorLabel(FString::Printf(TEXT("%s_%d"), *Variant.VariantName, FMath::RandRange(1, 999)));
        
        // Apply variant appearance
        UpdateCharacterAppearance(NewCharacter, Variant);
    }
    
    return NewCharacter;
}

void UChar_TribalCharacterManager::ApplyBiomeAdaptation(ACharacter* Character, EBiomeType BiomeType)
{
    if (!Character)
        return;
    
    // Find matching biome adaptation
    for (const FChar_BiomeAdaptation& Adaptation : BiomeAdaptations)
    {
        if (Adaptation.BiomeType == BiomeType)
        {
            // Apply movement speed modifier
            if (Character->GetCharacterMovement())
            {
                float BaseSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
                Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * Adaptation.MovementSpeedModifier;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Applied biome adaptation for %s: Speed modifier %.2f"), 
                   *Character->GetActorLabel(), Adaptation.MovementSpeedModifier);
            break;
        }
    }
}

void UChar_TribalCharacterManager::UpdateCharacterAppearance(ACharacter* Character, const FChar_TribalVariant& Variant)
{
    if (!Character)
        return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
        return;
    
    // Apply custom mesh if available
    if (Variant.CharacterMesh)
    {
        MeshComp->SetSkeletalMesh(Variant.CharacterMesh);
    }
    
    // Apply materials
    for (int32 i = 0; i < Variant.SkinMaterials.Num() && i < MeshComp->GetNumMaterials(); i++)
    {
        if (Variant.SkinMaterials[i])
        {
            MeshComp->SetMaterial(i, Variant.SkinMaterials[i]);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated appearance for character: %s"), *Character->GetActorLabel());
}