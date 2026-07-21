#include "Char_TribalCharacterManager.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AChar_TribalCharacterManager::AChar_TribalCharacterManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
}

void AChar_TribalCharacterManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager initialized with %d base meshes"), BaseMeshes.Num());
}

FChar_TribalAppearance AChar_TribalCharacterManager::GenerateRandomAppearance()
{
    FChar_TribalAppearance NewAppearance;
    
    NewAppearance.SkinTone = GetRandomSkinTone();
    NewAppearance.HairStyle = GetRandomHairStyle();
    NewAppearance.FacialMarkings = GetRandomFacialMarkings();
    NewAppearance.Height = GetRandomHeight();
    NewAppearance.Weight = GetRandomWeight();
    NewAppearance.Age = GetRandomAge();
    
    // Generate random name based on age and gender patterns
    TArray<FString> NamePrefixes = {TEXT("Kah"), TEXT("Mor"), TEXT("Tah"), TEXT("Grok"), TEXT("Uth"), TEXT("Nala"), TEXT("Kira"), TEXT("Zara")};
    TArray<FString> NameSuffixes = {TEXT("ak"), TEXT("un"), TEXT("ra"), TEXT("th"), TEXT("ka"), TEXT("na")};
    
    FString RandomPrefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    FString RandomSuffix = NameSuffixes[FMath::RandRange(0, NameSuffixes.Num() - 1)];
    NewAppearance.CharacterName = RandomPrefix + RandomSuffix;
    
    return NewAppearance;
}

FChar_TribalRole AChar_TribalCharacterManager::GetTribalRole(const FString& RoleName)
{
    FChar_TribalRole Role;
    
    if (RoleName == TEXT("Hunter"))
    {
        Role.RoleName = TEXT("Hunter");
        Role.EquipmentList = {TEXT("Stone Spear"), TEXT("Hide Clothing"), TEXT("Bone Knife")};
        Role.StrengthModifier = 1.2f;
        Role.AgilityModifier = 1.3f;
        Role.IntelligenceModifier = 1.0f;
    }
    else if (RoleName == TEXT("Gatherer"))
    {
        Role.RoleName = TEXT("Gatherer");
        Role.EquipmentList = {TEXT("Woven Basket"), TEXT("Hide Clothing"), TEXT("Digging Stick")};
        Role.StrengthModifier = 0.9f;
        Role.AgilityModifier = 1.1f;
        Role.IntelligenceModifier = 1.2f;
    }
    else if (RoleName == TEXT("Elder"))
    {
        Role.RoleName = TEXT("Elder");
        Role.EquipmentList = {TEXT("Wooden Staff"), TEXT("Decorated Hide"), TEXT("Bone Ornaments")};
        Role.StrengthModifier = 0.7f;
        Role.AgilityModifier = 0.8f;
        Role.IntelligenceModifier = 1.5f;
    }
    else if (RoleName == TEXT("Child"))
    {
        Role.RoleName = TEXT("Child");
        Role.EquipmentList = {TEXT("Simple Hide"), TEXT("Toy Spear")};
        Role.StrengthModifier = 0.4f;
        Role.AgilityModifier = 1.1f;
        Role.IntelligenceModifier = 0.8f;
    }
    else if (RoleName == TEXT("Crafter"))
    {
        Role.RoleName = TEXT("Crafter");
        Role.EquipmentList = {TEXT("Stone Tools"), TEXT("Hide Clothing"), TEXT("Bone Needles")};
        Role.StrengthModifier = 1.0f;
        Role.AgilityModifier = 1.2f;
        Role.IntelligenceModifier = 1.3f;
    }
    
    return Role;
}

APawn* AChar_TribalCharacterManager::SpawnTribalCharacter(const FVector& Location, const FRotator& Rotation, const FChar_TribalAppearance& Appearance, const FChar_TribalRole& Role)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // For now, spawn a basic character pawn - in full implementation this would use MetaHuman or custom character class
    UClass* CharacterClass = ACharacter::StaticClass();
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load character class"));
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    APawn* NewCharacter = GetWorld()->SpawnActor<APawn>(CharacterClass, Location, Rotation, SpawnParams);
    
    if (NewCharacter)
    {
        // Apply appearance
        if (ACharacter* Character = Cast<ACharacter>(NewCharacter))
        {
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                ApplyAppearanceToCharacter(MeshComp, Appearance);
            }
        }
        
        // Equip with role-specific items
        EquipCharacterWithRole(NewCharacter, Role);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned tribal character: %s with role: %s"), *Appearance.CharacterName, *Role.RoleName);
    }
    
    return NewCharacter;
}

void AChar_TribalCharacterManager::ApplyAppearanceToCharacter(USkeletalMeshComponent* MeshComponent, const FChar_TribalAppearance& Appearance)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply base mesh if available
    if (BaseMeshes.IsValidIndex(0))
    {
        MeshComponent->SetSkeletalMesh(BaseMeshes[0]);
    }
    
    // Apply skin material based on skin tone
    if (SkinMaterials.IsValidIndex(Appearance.SkinTone))
    {
        MeshComponent->SetMaterial(0, SkinMaterials[Appearance.SkinTone]);
    }
    
    // Scale character based on height and weight
    float HeightScale = Appearance.Height / 175.0f; // 175cm is baseline
    float WeightScale = FMath::Clamp(Appearance.Weight / 70.0f, 0.8f, 1.2f); // 70kg is baseline
    
    FVector NewScale = FVector(WeightScale, WeightScale, HeightScale);
    MeshComponent->SetWorldScale3D(NewScale);
    
    UE_LOG(LogTemp, Log, TEXT("Applied appearance to character: Height=%.1f, Weight=%.1f, SkinTone=%d"), 
           Appearance.Height, Appearance.Weight, Appearance.SkinTone);
}

void AChar_TribalCharacterManager::EquipCharacterWithRole(APawn* Character, const FChar_TribalRole& Role)
{
    if (!Character)
    {
        return;
    }
    
    // In a full implementation, this would attach weapon and clothing meshes to the character
    // For now, we log the equipment being applied
    for (const FString& Equipment : Role.EquipmentList)
    {
        UE_LOG(LogTemp, Log, TEXT("Equipping %s with: %s"), *Character->GetName(), *Equipment);
    }
    
    // Apply role modifiers to character stats (would integrate with survival system)
    UE_LOG(LogTemp, Log, TEXT("Applied role modifiers - Str:%.1f, Agi:%.1f, Int:%.1f"), 
           Role.StrengthModifier, Role.AgilityModifier, Role.IntelligenceModifier);
}

TArray<FString> AChar_TribalCharacterManager::GetAvailableRoles()
{
    return {TEXT("Hunter"), TEXT("Gatherer"), TEXT("Elder"), TEXT("Child"), TEXT("Crafter")};
}

void AChar_TribalCharacterManager::GenerateTribalCommunity(const FVector& CenterLocation, int32 PopulationSize, float SpawnRadius)
{
    TArray<FString> AvailableRoles = GetAvailableRoles();
    
    for (int32 i = 0; i < PopulationSize; i++)
    {
        // Generate random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Generate random appearance and role
        FChar_TribalAppearance Appearance = GenerateRandomAppearance();
        FString RandomRole = AvailableRoles[FMath::RandRange(0, AvailableRoles.Num() - 1)];
        FChar_TribalRole Role = GetTribalRole(RandomRole);
        
        // Spawn the character
        APawn* NewCharacter = SpawnTribalCharacter(SpawnLocation, FRotator::ZeroRotator, Appearance, Role);
        
        if (NewCharacter)
        {
            // Set a meaningful name for the actor
            NewCharacter->SetActorLabel(FString::Printf(TEXT("Tribal_%s_%s"), *Role.RoleName, *Appearance.CharacterName));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated tribal community of %d members at location %s"), 
           PopulationSize, *CenterLocation.ToString());
}

int32 AChar_TribalCharacterManager::GetRandomSkinTone()
{
    return FMath::RandRange(0, FMath::Max(0, SkinMaterials.Num() - 1));
}

int32 AChar_TribalCharacterManager::GetRandomHairStyle()
{
    return FMath::RandRange(0, FMath::Max(0, HairMeshes.Num() - 1));
}

int32 AChar_TribalCharacterManager::GetRandomFacialMarkings()
{
    return FMath::RandRange(0, 5); // 0-5 different marking patterns
}

float AChar_TribalCharacterManager::GetRandomHeight()
{
    return FMath::RandRange(150.0f, 190.0f); // Realistic human height range in cm
}

float AChar_TribalCharacterManager::GetRandomWeight()
{
    return FMath::RandRange(50.0f, 90.0f); // Realistic weight range for tribal humans in kg
}

int32 AChar_TribalCharacterManager::GetRandomAge()
{
    // Weighted age distribution - more young adults, fewer elderly
    float Random = FMath::FRand();
    if (Random < 0.1f) return FMath::RandRange(5, 15);   // Children
    else if (Random < 0.6f) return FMath::RandRange(16, 35); // Young adults
    else if (Random < 0.9f) return FMath::RandRange(36, 50); // Adults
    else return FMath::RandRange(51, 70); // Elders
}