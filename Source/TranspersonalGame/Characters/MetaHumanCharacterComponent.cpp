#include "MetaHumanCharacterComponent.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

UMetaHumanCharacterComponent::UMetaHumanCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMetaHumanCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetupMetaHumanComponents();
}

void UMetaHumanCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMetaHumanCharacterComponent::LoadCharacter(const FCharacterDefinition& NewCharacterDefinition)
{
    if (bIsLoading)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character is already loading, ignoring new load request"));
        return;
    }

    CharacterDefinition = NewCharacterDefinition;
    bIsLoading = true;
    PendingMeshLoads = 0;

    // Load MetaHuman mesh
    if (CharacterDefinition.MetaHumanMesh.IsValid())
    {
        IncrementPendingLoads();
        
        FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
        StreamableManager.RequestAsyncLoad(
            CharacterDefinition.MetaHumanMesh.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(this, &UMetaHumanCharacterComponent::OnMetaHumanMeshLoaded)
        );
    }

    // Load clothing meshes
    if (CharacterDefinition.Clothing.HeadGear.IsValid())
    {
        IncrementPendingLoads();
        FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
        StreamableManager.RequestAsyncLoad(
            CharacterDefinition.Clothing.HeadGear.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(this, &UMetaHumanCharacterComponent::OnClothingMeshLoaded)
        );
    }

    // If no meshes to load, complete immediately
    if (PendingMeshLoads == 0)
    {
        CheckLoadingComplete();
    }
}

void UMetaHumanCharacterComponent::LoadCharacterByName(const FString& CharacterName)
{
    if (!CharacterDatabase.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Character database not set"));
        return;
    }

    UCharacterDatabase* Database = CharacterDatabase.LoadSynchronous();
    if (!Database)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load character database"));
        return;
    }

    FCharacterDefinition Character = Database->GetCharacterByName(CharacterName);
    if (Character.CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Character not found: %s"), *CharacterName);
        return;
    }

    LoadCharacter(Character);
}

void UMetaHumanCharacterComponent::LoadRandomCharacterByArchetype(ECharacterArchetype Archetype)
{
    if (!CharacterDatabase.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Character database not set"));
        return;
    }

    UCharacterDatabase* Database = CharacterDatabase.LoadSynchronous();
    if (!Database)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load character database"));
        return;
    }

    FCharacterDefinition Character = Database->GetRandomCharacterByArchetype(Archetype);
    if (Character.CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No character found for archetype"));
        return;
    }

    LoadCharacter(Character);
}

void UMetaHumanCharacterComponent::ApplyPhysicalTraits(const FCharacterPhysicalTraits& Traits)
{
    // Apply physical traits to MetaHuman components
    if (BodyMeshComponent)
    {
        // Apply height and weight scaling
        FVector Scale = FVector(Traits.Weight, Traits.Weight, Traits.Height);
        BodyMeshComponent->SetWorldScale3D(Scale);

        // Apply skin tone, muscle mass, and body fat through material parameters
        UMaterialInstanceDynamic* BodyMaterial = BodyMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
        if (BodyMaterial)
        {
            BodyMaterial->SetVectorParameterValue(TEXT("SkinTone"), Traits.SkinTone);
            BodyMaterial->SetScalarParameterValue(TEXT("MuscleMass"), Traits.MuscleMass);
            BodyMaterial->SetScalarParameterValue(TEXT("BodyFat"), Traits.BodyFat);
        }
    }
}

void UMetaHumanCharacterComponent::ApplyClothing(const FCharacterClothing& Clothing)
{
    // Apply clothing meshes to components
    if (HeadGearComponent && Clothing.HeadGear.IsValid())
    {
        USkeletalMesh* HeadGearMesh = Clothing.HeadGear.LoadSynchronous();
        HeadGearComponent->SetSkeletalMesh(HeadGearMesh);
    }
}

void UMetaHumanCharacterComponent::SetupMetaHumanComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Find or create skeletal mesh components
    BodyMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    
    if (!BodyMeshComponent)
    {
        BodyMeshComponent = NewObject<USkeletalMeshComponent>(Owner, TEXT("BodyMesh"));
        Owner->AddInstanceComponent(BodyMeshComponent);
        BodyMeshComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void UMetaHumanCharacterComponent::SetMasterPoseComponent(USkeletalMeshComponent* MasterComponent)
{
    if (!MasterComponent)
    {
        return;
    }

    // Set all clothing components to follow the master pose
    if (HeadGearComponent) HeadGearComponent->SetLeaderPoseComponent(MasterComponent);
}

void UMetaHumanCharacterComponent::OnMetaHumanMeshLoaded()
{
    if (BodyMeshComponent && CharacterDefinition.MetaHumanMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = CharacterDefinition.MetaHumanMesh.LoadSynchronous();
        BodyMeshComponent->SetSkeletalMesh(LoadedMesh);
        
        // Apply physical traits
        ApplyPhysicalTraits(CharacterDefinition.PhysicalTraits);
    }

    DecrementPendingLoads();
}

void UMetaHumanCharacterComponent::OnClothingMeshLoaded()
{
    // Apply clothing
    ApplyClothing(CharacterDefinition.Clothing);
    
    DecrementPendingLoads();
}

void UMetaHumanCharacterComponent::IncrementPendingLoads()
{
    PendingMeshLoads++;
}

void UMetaHumanCharacterComponent::DecrementPendingLoads()
{
    PendingMeshLoads--;
    CheckLoadingComplete();
}

void UMetaHumanCharacterComponent::CheckLoadingComplete()
{
    if (PendingMeshLoads <= 0 && bIsLoading)
    {
        bIsLoading = false;
        OnCharacterLoaded.Broadcast(CharacterDefinition);
        
        UE_LOG(LogTemp, Log, TEXT("Character loading complete: %s"), *CharacterDefinition.CharacterName);
    }
}