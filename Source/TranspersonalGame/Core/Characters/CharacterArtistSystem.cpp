// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CharacterArtistSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UCharacterArtistSystem::UCharacterArtistSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Initialize default values
    MaxCharactersPerFrame = 5;
    CharacterLODDistance = 2000.0f;
    bEnableRealTimeUpdates = true;
    CharacterQualityLevel = ECharacterQuality::High;
    DiversityLevel = EDiversityLevel::High;
    
    // Initialize arrays
    ActiveCharacterRequests.Empty();
    CharacterTemplates.Empty();
    MaterialVariations.Empty();
}

void UCharacterArtistSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterArtistSystem: Failed to get world reference"));
        return;
    }

    // Initialize character creation system
    InitializeCharacterTemplates();
    SetupMaterialVariations();
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Initialized with %d character templates"), CharacterTemplates.Num());
}

void UCharacterArtistSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableRealTimeUpdates)
    {
        ProcessCharacterRequests(DeltaTime);
        UpdateCharacterLOD();
    }
}

void UCharacterArtistSystem::InitializeCharacterTemplates()
{
    // Create base character templates for the Jurassic survival game
    CharacterTemplates.Empty();
    
    // Paleontologist protagonist template
    FCharacterTemplate ProtagonistTemplate;
    ProtagonistTemplate.CharacterType = ECharacterType::Protagonist;
    ProtagonistTemplate.BaseMeshPath = TEXT("/Game/Characters/Protagonist/SK_Paleontologist");
    ProtagonistTemplate.MaterialPath = TEXT("/Game/Characters/Protagonist/M_Paleontologist");
    ProtagonistTemplate.AnimBlueprintPath = TEXT("/Game/Characters/Protagonist/ABP_Paleontologist");
    CharacterTemplates.Add(TEXT("Protagonist"), ProtagonistTemplate);
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Character templates initialized"));
}

void UCharacterArtistSystem::SetupMaterialVariations()
{
    // Setup material variations for character diversity
    MaterialVariations.Empty();
    
    // Skin tone variations
    TArray<FLinearColor> SkinTones = {
        FLinearColor(1.0f, 0.8f, 0.6f, 1.0f),  // Light
        FLinearColor(0.8f, 0.6f, 0.4f, 1.0f),  // Medium
        FLinearColor(0.6f, 0.4f, 0.3f, 1.0f),  // Dark
        FLinearColor(0.9f, 0.7f, 0.5f, 1.0f)   // Olive
    };
    
    MaterialVariations.Add(TEXT("SkinTones"), SkinTones);
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Material variations setup complete"));
}

void UCharacterArtistSystem::ProcessCharacterRequests(float DeltaTime)
{
    int32 ProcessedThisFrame = 0;
    
    for (int32 i = ActiveCharacterRequests.Num() - 1; i >= 0 && ProcessedThisFrame < MaxCharactersPerFrame; --i)
    {
        FCharacterCreationRequest& Request = ActiveCharacterRequests[i];
        
        if (ProcessCharacterRequest(Request))
        {
            ActiveCharacterRequests.RemoveAt(i);
            ProcessedThisFrame++;
        }
    }
}

bool UCharacterArtistSystem::ProcessCharacterRequest(const FCharacterCreationRequest& Request)
{
    // Process individual character creation request
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Processing character request for type %d"), 
           static_cast<int32>(Request.CharacterType));
    
    // This would contain the actual character creation logic
    return true;
}

void UCharacterArtistSystem::UpdateCharacterLOD()
{
    // Update character level of detail based on distance
    if (!CachedWorld)
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = CachedWorld->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for all tracked characters
    // This would iterate through all managed characters and adjust their LOD
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("CharacterArtistSystem: Updated character LOD from player location %s"), 
           *PlayerLocation.ToString());
}

AActor* UCharacterArtistSystem::CreateCharacterFromTemplate(const FString& TemplateName, 
                                                          FVector SpawnLocation, 
                                                          FRotator SpawnRotation)
{
    if (!CharacterTemplates.Contains(TemplateName))
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterArtistSystem: Template %s not found"), *TemplateName);
        return nullptr;
    }
    
    const FCharacterTemplate& Template = CharacterTemplates[TemplateName];
    
    // Create character creation request
    FCharacterCreationRequest NewRequest;
    NewRequest.CharacterType = Template.CharacterType;
    NewRequest.SpawnLocation = SpawnLocation;
    NewRequest.SpawnRotation = SpawnRotation;
    NewRequest.TemplateName = TemplateName;
    
    ActiveCharacterRequests.Add(NewRequest);
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Queued character creation for template %s"), *TemplateName);
    
    // Return nullptr for now, actual character would be created asynchronously
    return nullptr;
}

void UCharacterArtistSystem::ApplyCharacterVariation(AActor* Character, const FCharacterVariationData& VariationData)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterArtistSystem: Cannot apply variation to null character"));
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterArtistSystem: Character has no skeletal mesh component"));
        return;
    }
    
    // Apply material variations
    for (int32 MaterialIndex = 0; MaterialIndex < SkeletalMesh->GetNumMaterials(); ++MaterialIndex)
    {
        UMaterialInterface* BaseMaterial = SkeletalMesh->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply skin tone variation
                if (MaterialVariations.Contains(TEXT("SkinTones")) && MaterialVariations[TEXT("SkinTones")].Num() > 0)
                {
                    int32 SkinToneIndex = FMath::RandRange(0, MaterialVariations[TEXT("SkinTones")].Num() - 1);
                    DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), MaterialVariations[TEXT("SkinTones")][SkinToneIndex]);
                }
                
                SkeletalMesh->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Applied character variation to %s"), *Character->GetName());
}

void UCharacterArtistSystem::SetCharacterQuality(ECharacterQuality NewQuality)
{
    CharacterQualityLevel = NewQuality;
    
    // Adjust settings based on quality level
    switch (CharacterQualityLevel)
    {
        case ECharacterQuality::Low:
            MaxCharactersPerFrame = 10;
            CharacterLODDistance = 1000.0f;
            break;
        case ECharacterQuality::Medium:
            MaxCharactersPerFrame = 7;
            CharacterLODDistance = 1500.0f;
            break;
        case ECharacterQuality::High:
            MaxCharactersPerFrame = 5;
            CharacterLODDistance = 2000.0f;
            break;
        case ECharacterQuality::Ultra:
            MaxCharactersPerFrame = 3;
            CharacterLODDistance = 3000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Character quality set to %d"), static_cast<int32>(CharacterQualityLevel));
}

void UCharacterArtistSystem::SetDiversityLevel(EDiversityLevel NewDiversityLevel)
{
    DiversityLevel = NewDiversityLevel;
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Diversity level set to %d"), static_cast<int32>(DiversityLevel));
}

int32 UCharacterArtistSystem::GetActiveCharacterCount() const
{
    return ActiveCharacterRequests.Num();
}

void UCharacterArtistSystem::ClearAllCharacterRequests()
{
    ActiveCharacterRequests.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("CharacterArtistSystem: Cleared all character requests"));
}