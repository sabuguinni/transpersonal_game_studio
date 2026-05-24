#include "CharacterMeshManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimBlueprint.h"

UCharacterMeshManager::UCharacterMeshManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    CharacterType = EChar_CharacterType::Player;
    TargetMeshComponent = nullptr;
}

void UCharacterMeshManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindTargetMeshComponent();
    InitializeTribalPresets();
    
    // Load default mannequin mesh if no mesh is set
    if (TargetMeshComponent && !TargetMeshComponent->GetSkeletalMeshAsset())
    {
        LoadDefaultMannequinMesh();
    }
}

void UCharacterMeshManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterMeshManager::SetCharacterAppearance(const FChar_CharacterAppearance& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    UpdateCharacterMesh();
}

void UCharacterMeshManager::ApplyTribalVariation(EChar_CharacterType NewCharacterType)
{
    CharacterType = NewCharacterType;
    
    FChar_CharacterAppearance NewAppearance = CreateRandomTribalAppearance(CharacterType);
    SetCharacterAppearance(NewAppearance);
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal variation: %s"), 
           *UEnum::GetValueAsString(CharacterType));
}

void UCharacterMeshManager::SetSkinMaterial(UMaterialInterface* NewSkinMaterial)
{
    if (TargetMeshComponent && NewSkinMaterial)
    {
        TargetMeshComponent->SetMaterial(0, NewSkinMaterial);
        CurrentAppearance.SkinMaterial = NewSkinMaterial;
        
        UE_LOG(LogTemp, Log, TEXT("Applied skin material: %s"), 
               *NewSkinMaterial->GetName());
    }
}

void UCharacterMeshManager::SetClothingMaterial(UMaterialInterface* NewClothingMaterial)
{
    if (TargetMeshComponent && NewClothingMaterial)
    {
        // Apply to clothing material slots (typically slots 1-3)
        for (int32 i = 1; i < 4; i++)
        {
            if (TargetMeshComponent->GetMaterial(i))
            {
                TargetMeshComponent->SetMaterial(i, NewClothingMaterial);
            }
        }
        CurrentAppearance.ClothingMaterial = NewClothingMaterial;
        
        UE_LOG(LogTemp, Log, TEXT("Applied clothing material: %s"), 
               *NewClothingMaterial->GetName());
    }
}

void UCharacterMeshManager::RandomizeAppearance()
{
    // Create random tribal appearance based on current character type
    FChar_CharacterAppearance RandomAppearance = CreateRandomTribalAppearance(CharacterType);
    SetCharacterAppearance(RandomAppearance);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized appearance for character: %s"), 
           *RandomAppearance.CharacterName);
}

bool UCharacterMeshManager::LoadDefaultMannequinMesh()
{
    if (!TargetMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No target mesh component found"));
        return false;
    }
    
    // Try to load UE5 Mannequin mesh
    FString MannequinPath = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple");
    USkeletalMesh* MannequinMesh = LoadObject<USkeletalMesh>(nullptr, *MannequinPath);
    
    if (!MannequinMesh)
    {
        // Try alternative ThirdPerson template mesh
        MannequinPath = TEXT("/Game/ThirdPerson/Meshes/SK_Mannequin");
        MannequinMesh = LoadObject<USkeletalMesh>(nullptr, *MannequinPath);
    }
    
    if (MannequinMesh)
    {
        TargetMeshComponent->SetSkeletalMesh(MannequinMesh);
        CurrentAppearance.BaseMesh = MannequinMesh;
        
        UE_LOG(LogTemp, Log, TEXT("Loaded mannequin mesh: %s"), *MannequinPath);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Failed to load mannequin mesh"));
    return false;
}

void UCharacterMeshManager::UpdateCharacterMesh()
{
    if (!TargetMeshComponent)
    {
        return;
    }
    
    // Apply base mesh
    if (CurrentAppearance.BaseMesh.IsValid())
    {
        USkeletalMesh* Mesh = CurrentAppearance.BaseMesh.LoadSynchronous();
        if (Mesh)
        {
            TargetMeshComponent->SetSkeletalMesh(Mesh);
        }
    }
    
    // Apply materials
    if (CurrentAppearance.SkinMaterial.IsValid())
    {
        UMaterialInterface* SkinMat = CurrentAppearance.SkinMaterial.LoadSynchronous();
        if (SkinMat)
        {
            TargetMeshComponent->SetMaterial(0, SkinMat);
        }
    }
    
    if (CurrentAppearance.ClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = CurrentAppearance.ClothingMaterial.LoadSynchronous();
        if (ClothingMat)
        {
            for (int32 i = 1; i < 4; i++)
            {
                if (TargetMeshComponent->GetMaterial(i))
                {
                    TargetMeshComponent->SetMaterial(i, ClothingMat);
                }
            }
        }
    }
    
    // Apply animation blueprint
    if (CurrentAppearance.AnimationBlueprint.IsValid())
    {
        UAnimBlueprint* AnimBP = CurrentAppearance.AnimationBlueprint.LoadSynchronous();
        if (AnimBP && AnimBP->GeneratedClass)
        {
            TargetMeshComponent->SetAnimInstanceClass(AnimBP->GeneratedClass);
        }
    }
}

void UCharacterMeshManager::InitializeTribalPresets()
{
    // Initialize tribal hunter presets
    TribalHunterPresets.Empty();
    for (int32 i = 0; i < 3; i++)
    {
        FChar_CharacterAppearance HunterPreset;
        HunterPreset.CharacterName = FString::Printf(TEXT("Hunter_%d"), i + 1);
        HunterPreset.TribalAffiliation = TEXT("Stone Spear Tribe");
        TribalHunterPresets.Add(HunterPreset);
    }
    
    // Initialize tribal gatherer presets
    TribalGathererPresets.Empty();
    for (int32 i = 0; i < 3; i++)
    {
        FChar_CharacterAppearance GathererPreset;
        GathererPreset.CharacterName = FString::Printf(TEXT("Gatherer_%d"), i + 1);
        GathererPreset.TribalAffiliation = TEXT("Plant Weaver Tribe");
        TribalGathererPresets.Add(GathererPreset);
    }
    
    // Initialize tribal elder presets
    TribalElderPresets.Empty();
    for (int32 i = 0; i < 2; i++)
    {
        FChar_CharacterAppearance ElderPreset;
        ElderPreset.CharacterName = FString::Printf(TEXT("Elder_%d"), i + 1);
        ElderPreset.TribalAffiliation = TEXT("Wisdom Keeper Tribe");
        TribalElderPresets.Add(ElderPreset);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized tribal appearance presets"));
}

void UCharacterMeshManager::FindTargetMeshComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Try to find skeletal mesh component on owner
    TargetMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    
    if (!TargetMeshComponent)
    {
        // If owner is a Character, get the mesh component
        ACharacter* Character = Cast<ACharacter>(Owner);
        if (Character)
        {
            TargetMeshComponent = Character->GetMesh();
        }
    }
    
    if (TargetMeshComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Found target mesh component: %s"), 
               *TargetMeshComponent->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh component found on owner"));
    }
}

FChar_CharacterAppearance UCharacterMeshManager::CreateRandomTribalAppearance(EChar_CharacterType Type)
{
    FChar_CharacterAppearance RandomAppearance;
    
    switch (Type)
    {
        case EChar_CharacterType::TribalHunter:
            if (TribalHunterPresets.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, TribalHunterPresets.Num() - 1);
                RandomAppearance = TribalHunterPresets[RandomIndex];
            }
            break;
            
        case EChar_CharacterType::TribalGatherer:
            if (TribalGathererPresets.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, TribalGathererPresets.Num() - 1);
                RandomAppearance = TribalGathererPresets[RandomIndex];
            }
            break;
            
        case EChar_CharacterType::TribalElder:
            if (TribalElderPresets.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, TribalElderPresets.Num() - 1);
                RandomAppearance = TribalElderPresets[RandomIndex];
            }
            break;
            
        default:
            RandomAppearance.CharacterName = TEXT("Default_Character");
            RandomAppearance.TribalAffiliation = TEXT("Wanderer");
            break;
    }
    
    return RandomAppearance;
}