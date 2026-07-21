#include "TribalCharacterSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"

UTribalCharacterSystem::UTribalCharacterSystem()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize default appearance
    CurrentAppearance = FChar_TribalAppearance();
    
    // Setup default configurations
    SetupDefaultClothingSets();
    SetupDefaultSkinMaterials();
    SetupDefaultBodyScales();
}

void UTribalCharacterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply default appearance on start
    InitializeDefaultAppearance();
}

void UTribalCharacterSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTribalCharacterSystem::ApplyTribalAppearance(const FChar_TribalAppearance& Appearance)
{
    CurrentAppearance = Appearance;
    
    // Apply all appearance changes
    ApplySkinMaterial();
    ApplyClothingMaterials();
    ApplyBodyScaling();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal appearance for %s"), *Appearance.CharacterName);
}

void UTribalCharacterSystem::RandomizeAppearance()
{
    FChar_TribalAppearance RandomAppearance = GenerateRandomAppearance();
    ApplyTribalAppearance(RandomAppearance);
}

void UTribalCharacterSystem::SetTribalRole(EChar_TribalRole NewRole)
{
    CurrentAppearance.Role = NewRole;
    ApplyClothingMaterials();
}

void UTribalCharacterSystem::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    ApplySkinMaterial();
}

void UTribalCharacterSystem::SetBodyType(EChar_BodyType NewBodyType)
{
    CurrentAppearance.BodyType = NewBodyType;
    ApplyBodyScaling();
}

FChar_TribalAppearance UTribalCharacterSystem::GenerateRandomAppearance()
{
    FChar_TribalAppearance RandomAppearance;
    
    // Randomize role
    int32 RoleIndex = FMath::RandRange(0, 4);
    RandomAppearance.Role = static_cast<EChar_TribalRole>(RoleIndex);
    
    // Randomize skin tone
    int32 SkinIndex = FMath::RandRange(0, 5);
    RandomAppearance.SkinTone = static_cast<EChar_SkinTone>(SkinIndex);
    
    // Randomize body type
    int32 BodyIndex = FMath::RandRange(0, 3);
    RandomAppearance.BodyType = static_cast<EChar_BodyType>(BodyIndex);
    
    // Randomize gender
    RandomAppearance.bIsMale = FMath::RandBool();
    
    // Randomize physical attributes
    RandomAppearance.Height = FMath::RandRange(1.5f, 1.9f);
    RandomAppearance.Weight = FMath::RandRange(50.0f, 90.0f);
    RandomAppearance.Age = FMath::RandRange(18, 60);
    
    // Generate name based on role and gender
    FString BaseName;
    switch (RandomAppearance.Role)
    {
        case EChar_TribalRole::Hunter:
            BaseName = RandomAppearance.bIsMale ? TEXT("Kael") : TEXT("Nara");
            break;
        case EChar_TribalRole::Gatherer:
            BaseName = RandomAppearance.bIsMale ? TEXT("Finn") : TEXT("Vera");
            break;
        case EChar_TribalRole::Crafter:
            BaseName = RandomAppearance.bIsMale ? TEXT("Thane") : TEXT("Lyra");
            break;
        case EChar_TribalRole::Elder:
            BaseName = RandomAppearance.bIsMale ? TEXT("Aldric") : TEXT("Mira");
            break;
        case EChar_TribalRole::Child:
            BaseName = RandomAppearance.bIsMale ? TEXT("Pip") : TEXT("Zara");
            break;
        default:
            BaseName = TEXT("Tribal");
            break;
    }
    
    RandomAppearance.CharacterName = FString::Printf(TEXT("%s_%d"), *BaseName, FMath::RandRange(1, 999));
    
    return RandomAppearance;
}

void UTribalCharacterSystem::ApplySkinMaterial()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }
    
    if (SkinMaterials.Contains(CurrentAppearance.SkinTone))
    {
        UMaterialInterface* SkinMaterial = SkinMaterials[CurrentAppearance.SkinTone].LoadSynchronous();
        if (SkinMaterial)
        {
            ApplyMaterialToMeshSlot(SkinMaterial, 0); // Assuming skin is slot 0
            UE_LOG(LogTemp, Log, TEXT("Applied skin material for skin tone %d"), (int32)CurrentAppearance.SkinTone);
        }
    }
}

void UTribalCharacterSystem::ApplyClothingMaterials()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }
    
    if (ClothingSets.Contains(CurrentAppearance.Role))
    {
        const FChar_ClothingSet& ClothingSet = ClothingSets[CurrentAppearance.Role];
        
        // Apply chest material (slot 1)
        if (UMaterialInterface* ChestMat = ClothingSet.ChestMaterial.LoadSynchronous())
        {
            ApplyMaterialToMeshSlot(ChestMat, 1);
        }
        
        // Apply legs material (slot 2)
        if (UMaterialInterface* LegsMat = ClothingSet.LegsMaterial.LoadSynchronous())
        {
            ApplyMaterialToMeshSlot(LegsMat, 2);
        }
        
        // Apply feet material (slot 3)
        if (UMaterialInterface* FeetMat = ClothingSet.FeetMaterial.LoadSynchronous())
        {
            ApplyMaterialToMeshSlot(FeetMat, 3);
        }
        
        // Apply accessory material (slot 4)
        if (UMaterialInterface* AccessoryMat = ClothingSet.AccessoryMaterial.LoadSynchronous())
        {
            ApplyMaterialToMeshSlot(AccessoryMat, 4);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Applied clothing materials for role %d"), (int32)CurrentAppearance.Role);
    }
}

void UTribalCharacterSystem::ApplyBodyScaling()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }
    
    if (BodyScales.Contains(CurrentAppearance.BodyType))
    {
        FVector Scale = BodyScales[CurrentAppearance.BodyType];
        
        // Apply height scaling
        Scale.Z *= (CurrentAppearance.Height / 1.75f); // Normalize to default height
        
        // Apply weight scaling (affects X and Y)
        float WeightFactor = CurrentAppearance.Weight / 70.0f; // Normalize to default weight
        Scale.X *= WeightFactor;
        Scale.Y *= WeightFactor;
        
        MeshComp->SetWorldScale3D(Scale);
        
        UE_LOG(LogTemp, Log, TEXT("Applied body scaling: %s"), *Scale.ToString());
    }
}

USkeletalMeshComponent* UTribalCharacterSystem::GetCharacterMesh()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }
    
    // Try to get from Character class first
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        return Character->GetMesh();
    }
    
    // Fallback to component search
    return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UTribalCharacterSystem::InitializeDefaultAppearance()
{
    // Apply the current appearance settings
    ApplyTribalAppearance(CurrentAppearance);
}

void UTribalCharacterSystem::SetupDefaultClothingSets()
{
    // Hunter clothing set - minimal leather
    FChar_ClothingSet HunterSet;
    HunterSet.ClothingName = TEXT("Hunter Gear");
    ClothingSets.Add(EChar_TribalRole::Hunter, HunterSet);
    
    // Gatherer clothing set - practical woven materials
    FChar_ClothingSet GathererSet;
    GathererSet.ClothingName = TEXT("Gatherer Robes");
    ClothingSets.Add(EChar_TribalRole::Gatherer, GathererSet);
    
    // Crafter clothing set - tool belts and aprons
    FChar_ClothingSet CrafterSet;
    CrafterSet.ClothingName = TEXT("Crafter Attire");
    ClothingSets.Add(EChar_TribalRole::Crafter, CrafterSet);
    
    // Elder clothing set - decorated robes
    FChar_ClothingSet ElderSet;
    ElderSet.ClothingName = TEXT("Elder Vestments");
    ClothingSets.Add(EChar_TribalRole::Elder, ElderSet);
    
    // Child clothing set - simple wraps
    FChar_ClothingSet ChildSet;
    ChildSet.ClothingName = TEXT("Child Wraps");
    ClothingSets.Add(EChar_TribalRole::Child, ChildSet);
}

void UTribalCharacterSystem::SetupDefaultSkinMaterials()
{
    // Note: These would be set to actual material paths in a real implementation
    // For now, we just initialize the map structure
    SkinMaterials.Add(EChar_SkinTone::Pale, nullptr);
    SkinMaterials.Add(EChar_SkinTone::Fair, nullptr);
    SkinMaterials.Add(EChar_SkinTone::Olive, nullptr);
    SkinMaterials.Add(EChar_SkinTone::Bronze, nullptr);
    SkinMaterials.Add(EChar_SkinTone::Dark, nullptr);
    SkinMaterials.Add(EChar_SkinTone::Deep, nullptr);
}

void UTribalCharacterSystem::SetupDefaultBodyScales()
{
    // Define scaling factors for different body types
    BodyScales.Add(EChar_BodyType::Lean, FVector(0.9f, 0.9f, 1.0f));
    BodyScales.Add(EChar_BodyType::Athletic, FVector(1.0f, 1.0f, 1.0f));
    BodyScales.Add(EChar_BodyType::Muscular, FVector(1.1f, 1.1f, 1.05f));
    BodyScales.Add(EChar_BodyType::Heavy, FVector(1.2f, 1.2f, 1.0f));
}

void UTribalCharacterSystem::ApplyMaterialToMeshSlot(UMaterialInterface* Material, int32 SlotIndex)
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (MeshComp && Material)
    {
        MeshComp->SetMaterial(SlotIndex, Material);
    }
}