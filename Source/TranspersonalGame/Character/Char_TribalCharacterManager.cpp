#include "Char_TribalCharacterManager.h"
#include "TranspersonalCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AChar_TribalCharacterManager::AChar_TribalCharacterManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MaxCharactersPerBiome = 20;
    CharacterSpawnRadius = 5000.0f;
    
    // Initialize component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AChar_TribalCharacterManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCharacterTemplates();
    InitializeBiomeTemplates();
    InitializeRoleDefaults();
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Initialized with %d character templates"), CharacterTemplates.Num());
}

void AChar_TribalCharacterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

ATranspersonalCharacter* AChar_TribalCharacterManager::CreateTribalCharacter(const FChar_TribalAppearance& Appearance, EChar_TribalRole Role, const FVector& SpawnLocation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("TribalCharacterManager: No valid world for character spawn"));
        return nullptr;
    }

    // Spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // Spawn the character
    ATranspersonalCharacter* NewCharacter = GetWorld()->SpawnActor<ATranspersonalCharacter>(ATranspersonalCharacter::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    
    if (NewCharacter)
    {
        // Apply appearance customization
        ApplyBiomeAdaptation(NewCharacter, Appearance.AdaptedBiome);
        SetCharacterRole(NewCharacter, Role);
        
        // Generate personality based on role
        FChar_PersonalityTraits Personality = GenerateRandomPersonality();
        if (RolePersonalityDefaults.Contains(Role))
        {
            FChar_PersonalityTraits RoleDefault = RolePersonalityDefaults[Role];
            // Blend random with role default (70% role, 30% random)
            Personality.Courage = (RoleDefault.Courage * 0.7f) + (Personality.Courage * 0.3f);
            Personality.Aggression = (RoleDefault.Aggression * 0.7f) + (Personality.Aggression * 0.3f);
            Personality.Intelligence = (RoleDefault.Intelligence * 0.7f) + (Personality.Intelligence * 0.3f);
            Personality.Social = (RoleDefault.Social * 0.7f) + (Personality.Social * 0.3f);
            Personality.Curiosity = (RoleDefault.Curiosity * 0.7f) + (Personality.Curiosity * 0.3f);
        }
        
        ApplyPersonalityTraits(NewCharacter, Personality);
        
        // Set character label for identification
        FString CharacterLabel = FString::Printf(TEXT("Tribal_%s_%s"), 
            *UEnum::GetValueAsString(Role),
            *UEnum::GetValueAsString(Appearance.AdaptedBiome));
        NewCharacter->SetActorLabel(CharacterLabel);
        
        // Track the character
        ManagedCharacters.Add(NewCharacter);
        
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Created %s at %s"), *CharacterLabel, *SpawnLocation.ToString());
        
        return NewCharacter;
    }
    
    UE_LOG(LogTemp, Error, TEXT("TribalCharacterManager: Failed to spawn character"));
    return nullptr;
}

void AChar_TribalCharacterManager::GenerateRandomTribalCharacter(EBiomeType TargetBiome, const FVector& SpawnLocation)
{
    FChar_TribalAppearance RandomAppearance = GenerateRandomAppearance(TargetBiome);
    
    // Random role selection with biome preferences
    TArray<EChar_TribalRole> PossibleRoles;
    
    switch (TargetBiome)
    {
        case EBiomeType::Savanna:
            PossibleRoles = {EChar_TribalRole::Hunter, EChar_TribalRole::Scout, EChar_TribalRole::Gatherer};
            break;
        case EBiomeType::Swamp:
            PossibleRoles = {EChar_TribalRole::Gatherer, EChar_TribalRole::Crafter, EChar_TribalRole::Shaman};
            break;
        case EBiomeType::Forest:
            PossibleRoles = {EChar_TribalRole::Hunter, EChar_TribalRole::Gatherer, EChar_TribalRole::Crafter};
            break;
        case EBiomeType::Desert:
            PossibleRoles = {EChar_TribalRole::Scout, EChar_TribalRole::Hunter, EChar_TribalRole::Elder};
            break;
        case EBiomeType::Mountain:
            PossibleRoles = {EChar_TribalRole::Scout, EChar_TribalRole::Crafter, EChar_TribalRole::Elder};
            break;
        default:
            PossibleRoles = {EChar_TribalRole::Hunter, EChar_TribalRole::Gatherer};
            break;
    }
    
    EChar_TribalRole SelectedRole = PossibleRoles[FMath::RandRange(0, PossibleRoles.Num() - 1)];
    CreateTribalCharacter(RandomAppearance, SelectedRole, SpawnLocation);
}

void AChar_TribalCharacterManager::CreateTribalGroup(EBiomeType Biome, const FVector& CenterLocation, int32 GroupSize)
{
    for (int32 i = 0; i < GroupSize; i++)
    {
        // Random position around center
        float Angle = (2.0f * PI * i) / GroupSize;
        float Distance = FMath::RandRange(100.0f, 500.0f);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        GenerateRandomTribalCharacter(Biome, SpawnLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Created tribal group of %d characters in %s biome"), 
        GroupSize, *UEnum::GetValueAsString(Biome));
}

void AChar_TribalCharacterManager::ApplyBiomeAdaptation(ATranspersonalCharacter* Character, EBiomeType Biome)
{
    if (!Character)
    {
        return;
    }
    
    // Apply biome-specific adaptations
    FLinearColor AdaptedSkinTone = GetBiomeAdaptedSkinTone(Biome);
    FString BiomeClothing = GetBiomeSpecificClothing(Biome);
    
    // Log the adaptation
    UE_LOG(LogTemp, Log, TEXT("TribalCharacterManager: Applied %s biome adaptation to character"), 
        *UEnum::GetValueAsString(Biome));
}

void AChar_TribalCharacterManager::SetCharacterRole(ATranspersonalCharacter* Character, EChar_TribalRole Role)
{
    if (!Character)
    {
        return;
    }
    
    // Apply role-specific equipment and stats
    FString RoleWeapon = GetRoleSpecificWeapon(Role);
    TArray<FString> RoleAccessories = GetRoleSpecificAccessories(Role);
    
    UE_LOG(LogTemp, Log, TEXT("TribalCharacterManager: Set character role to %s"), 
        *UEnum::GetValueAsString(Role));
}

void AChar_TribalCharacterManager::ApplyPersonalityTraits(ATranspersonalCharacter* Character, const FChar_PersonalityTraits& Traits)
{
    if (!Character)
    {
        return;
    }
    
    // Apply personality traits to character behavior
    UE_LOG(LogTemp, Log, TEXT("TribalCharacterManager: Applied personality traits (Courage: %.2f, Aggression: %.2f, Intelligence: %.2f)"), 
        Traits.Courage, Traits.Aggression, Traits.Intelligence);
}

FChar_TribalAppearance AChar_TribalCharacterManager::GenerateRandomAppearance(EBiomeType Biome)
{
    FChar_TribalAppearance Appearance;
    
    // Random physical characteristics
    Appearance.Height = FMath::RandRange(150.0f, 190.0f);
    Appearance.Weight = FMath::RandRange(50.0f, 90.0f);
    
    // Biome-adapted appearance
    Appearance.SkinTone = GetBiomeAdaptedSkinTone(Biome);
    Appearance.HairColor = FLinearColor(
        FMath::RandRange(0.1f, 0.4f),
        FMath::RandRange(0.1f, 0.3f),
        FMath::RandRange(0.05f, 0.2f),
        1.0f
    );
    
    Appearance.ClothingType = GetBiomeSpecificClothing(Biome);
    Appearance.AdaptedBiome = Biome;
    
    return Appearance;
}

FChar_PersonalityTraits AChar_TribalCharacterManager::GenerateRandomPersonality()
{
    FChar_PersonalityTraits Traits;
    
    Traits.Courage = FMath::RandRange(0.2f, 0.9f);
    Traits.Aggression = FMath::RandRange(0.1f, 0.7f);
    Traits.Intelligence = FMath::RandRange(0.3f, 0.9f);
    Traits.Social = FMath::RandRange(0.2f, 0.8f);
    Traits.Curiosity = FMath::RandRange(0.1f, 0.8f);
    
    return Traits;
}

void AChar_TribalCharacterManager::TestCharacterCreation()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("TribalCharacterManager: No world for testing"));
        return;
    }
    
    FVector TestLocation = GetActorLocation() + FVector(500.0f, 0.0f, 0.0f);
    FChar_TribalAppearance TestAppearance = GenerateRandomAppearance(EBiomeType::Savanna);
    
    ATranspersonalCharacter* TestCharacter = CreateTribalCharacter(TestAppearance, EChar_TribalRole::Hunter, TestLocation);
    
    if (TestCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Test character created successfully"));
    }
}

void AChar_TribalCharacterManager::SpawnTestCharactersInAllBiomes()
{
    // Biome coordinates from memory
    TArray<TPair<EBiomeType, FVector>> BiomeLocations = {
        {EBiomeType::Savanna, FVector(0.0f, 0.0f, 100.0f)},
        {EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 100.0f)},
        {EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 100.0f)},
        {EBiomeType::Desert, FVector(55000.0f, 0.0f, 100.0f)},
        {EBiomeType::Mountain, FVector(40000.0f, 50000.0f, 100.0f)}
    };
    
    for (const auto& BiomeLocation : BiomeLocations)
    {
        CreateTribalGroup(BiomeLocation.Key, BiomeLocation.Value, 3);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TribalCharacterManager: Spawned test characters in all biomes"));
}

void AChar_TribalCharacterManager::InitializeCharacterTemplates()
{
    CharacterTemplates.Empty();
    
    // Create base templates for different character archetypes
    for (int32 i = 0; i < 5; i++)
    {
        FChar_TribalAppearance Template;
        Template.Height = 160.0f + (i * 5.0f);
        Template.Weight = 60.0f + (i * 4.0f);
        Template.SkinTone = FLinearColor(0.7f + (i * 0.05f), 0.5f + (i * 0.05f), 0.3f + (i * 0.05f), 1.0f);
        CharacterTemplates.Add(Template);
    }
}

void AChar_TribalCharacterManager::InitializeBiomeTemplates()
{
    BiomeSpecificTemplates.Empty();
    
    // Savanna template - sun-adapted
    FChar_TribalAppearance SavannaTemplate;
    SavannaTemplate.SkinTone = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    SavannaTemplate.ClothingType = TEXT("Light Animal Hide");
    BiomeSpecificTemplates.Add(EBiomeType::Savanna, SavannaTemplate);
    
    // Swamp template - moisture-adapted
    FChar_TribalAppearance SwampTemplate;
    SwampTemplate.SkinTone = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
    SwampTemplate.ClothingType = TEXT("Waterproof Plant Fiber");
    BiomeSpecificTemplates.Add(EBiomeType::Swamp, SwampTemplate);
    
    // Forest template - camouflage-adapted
    FChar_TribalAppearance ForestTemplate;
    ForestTemplate.SkinTone = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    ForestTemplate.ClothingType = TEXT("Leaf-Woven Garments");
    BiomeSpecificTemplates.Add(EBiomeType::Forest, ForestTemplate);
    
    // Desert template - heat-adapted
    FChar_TribalAppearance DesertTemplate;
    DesertTemplate.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    DesertTemplate.ClothingType = TEXT("Protective Wraps");
    BiomeSpecificTemplates.Add(EBiomeType::Desert, DesertTemplate);
    
    // Mountain template - cold-adapted
    FChar_TribalAppearance MountainTemplate;
    MountainTemplate.SkinTone = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    MountainTemplate.ClothingType = TEXT("Thick Fur Clothing");
    BiomeSpecificTemplates.Add(EBiomeType::Mountain, MountainTemplate);
}

void AChar_TribalCharacterManager::InitializeRoleDefaults()
{
    RolePersonalityDefaults.Empty();
    
    // Hunter - high courage, moderate aggression
    FChar_PersonalityTraits HunterTraits;
    HunterTraits.Courage = 0.8f;
    HunterTraits.Aggression = 0.6f;
    HunterTraits.Intelligence = 0.6f;
    HunterTraits.Social = 0.5f;
    HunterTraits.Curiosity = 0.4f;
    RolePersonalityDefaults.Add(EChar_TribalRole::Hunter, HunterTraits);
    
    // Gatherer - high intelligence, low aggression
    FChar_PersonalityTraits GathererTraits;
    GathererTraits.Courage = 0.5f;
    GathererTraits.Aggression = 0.2f;
    GathererTraits.Intelligence = 0.8f;
    GathererTraits.Social = 0.7f;
    GathererTraits.Curiosity = 0.9f;
    RolePersonalityDefaults.Add(EChar_TribalRole::Gatherer, GathererTraits);
    
    // Crafter - high intelligence, low aggression
    FChar_PersonalityTraits CrafterTraits;
    CrafterTraits.Courage = 0.4f;
    CrafterTraits.Aggression = 0.2f;
    CrafterTraits.Intelligence = 0.9f;
    CrafterTraits.Social = 0.6f;
    CrafterTraits.Curiosity = 0.8f;
    RolePersonalityDefaults.Add(EChar_TribalRole::Crafter, CrafterTraits);
    
    // Elder - high intelligence and social
    FChar_PersonalityTraits ElderTraits;
    ElderTraits.Courage = 0.7f;
    ElderTraits.Aggression = 0.3f;
    ElderTraits.Intelligence = 0.9f;
    ElderTraits.Social = 0.9f;
    ElderTraits.Curiosity = 0.6f;
    RolePersonalityDefaults.Add(EChar_TribalRole::Elder, ElderTraits);
    
    // Scout - high courage and curiosity
    FChar_PersonalityTraits ScoutTraits;
    ScoutTraits.Courage = 0.9f;
    ScoutTraits.Aggression = 0.4f;
    ScoutTraits.Intelligence = 0.7f;
    ScoutTraits.Social = 0.5f;
    ScoutTraits.Curiosity = 0.9f;
    RolePersonalityDefaults.Add(EChar_TribalRole::Scout, ScoutTraits);
}

FLinearColor AChar_TribalCharacterManager::GetBiomeAdaptedSkinTone(EBiomeType Biome)
{
    switch (Biome)
    {
        case EBiomeType::Savanna:
            return FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Darker, sun-adapted
        case EBiomeType::Swamp:
            return FLinearColor(0.5f, 0.4f, 0.3f, 1.0f); // Olive-toned
        case EBiomeType::Forest:
            return FLinearColor(0.7f, 0.5f, 0.3f, 1.0f); // Medium brown
        case EBiomeType::Desert:
            return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Tan, heat-adapted
        case EBiomeType::Mountain:
            return FLinearColor(0.9f, 0.7f, 0.5f, 1.0f); // Lighter, cold-adapted
        default:
            return FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    }
}

FString AChar_TribalCharacterManager::GetBiomeSpecificClothing(EBiomeType Biome)
{
    switch (Biome)
    {
        case EBiomeType::Savanna:
            return TEXT("Light Animal Hide");
        case EBiomeType::Swamp:
            return TEXT("Waterproof Plant Fiber");
        case EBiomeType::Forest:
            return TEXT("Leaf-Woven Garments");
        case EBiomeType::Desert:
            return TEXT("Protective Wraps");
        case EBiomeType::Mountain:
            return TEXT("Thick Fur Clothing");
        default:
            return TEXT("Basic Animal Hide");
    }
}

FString AChar_TribalCharacterManager::GetRoleSpecificWeapon(EChar_TribalRole Role)
{
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
            return TEXT("Stone-Tipped Spear");
        case EChar_TribalRole::Gatherer:
            return TEXT("Digging Stick");
        case EChar_TribalRole::Crafter:
            return TEXT("Stone Knife");
        case EChar_TribalRole::Elder:
            return TEXT("Ceremonial Staff");
        case EChar_TribalRole::Scout:
            return TEXT("Throwing Stones");
        case EChar_TribalRole::Shaman:
            return TEXT("Ritual Bone");
        case EChar_TribalRole::Child:
            return TEXT("Small Stick");
        default:
            return TEXT("Stone Tool");
    }
}

TArray<FString> AChar_TribalCharacterManager::GetRoleSpecificAccessories(EChar_TribalRole Role)
{
    TArray<FString> Accessories;
    
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
            Accessories.Add(TEXT("Bone Necklace"));
            Accessories.Add(TEXT("Feather Headdress"));
            break;
        case EChar_TribalRole::Gatherer:
            Accessories.Add(TEXT("Woven Basket"));
            Accessories.Add(TEXT("Seed Pouch"));
            break;
        case EChar_TribalRole::Crafter:
            Accessories.Add(TEXT("Tool Belt"));
            Accessories.Add(TEXT("Stone Collection"));
            break;
        case EChar_TribalRole::Elder:
            Accessories.Add(TEXT("Wisdom Beads"));
            Accessories.Add(TEXT("Elder Markings"));
            break;
        case EChar_TribalRole::Scout:
            Accessories.Add(TEXT("Travel Pack"));
            Accessories.Add(TEXT("Direction Stones"));
            break;
        case EChar_TribalRole::Shaman:
            Accessories.Add(TEXT("Ritual Paint"));
            Accessories.Add(TEXT("Sacred Symbols"));
            break;
        default:
            break;
    }
    
    return Accessories;
}