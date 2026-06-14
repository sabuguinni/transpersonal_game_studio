#include "Anim_MontageController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_MontageController::UAnim_MontageController()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsPlayingMontage = false;
    CurrentMontage = NAME_None;
    CurrentMontagePriority = -1;
    OwnerCharacter = nullptr;
    AnimInstance = nullptr;
}

void UAnim_MontageController::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MontageController: Owner is not a Character"));
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (MeshComp)
    {
        AnimInstance = MeshComp->GetAnimInstance();
        if (AnimInstance)
        {
            // Bind montage delegates
            AnimInstance->OnMontageEnded.AddDynamic(this, &UAnim_MontageController::OnMontageEnded);
            AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAnim_MontageController::OnMontageBlendingOut);
        }
    }
}

bool UAnim_MontageController::PlayMontage(FName MontageName, float PlayRate, bool bForcePlay)
{
    if (!AnimInstance || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MontageController: Invalid AnimInstance or Character"));
        return false;
    }

    FAnim_MontageData* MontageData = FindMontageData(MontageName);
    if (!MontageData)
    {
        UE_LOG(LogTemp, Warning, TEXT("MontageController: Montage '%s' not found in database"), *MontageName.ToString());
        return false;
    }

    if (!CanPlayMontage(*MontageData, bForcePlay))
    {
        UE_LOG(LogTemp, Log, TEXT("MontageController: Cannot play montage '%s' - higher priority montage playing"), *MontageName.ToString());
        return false;
    }

    // Load the montage asset
    UAnimMontage* Montage = MontageData->Montage.LoadSynchronous();
    if (!Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("MontageController: Failed to load montage asset for '%s'"), *MontageName.ToString());
        return false;
    }

    // Stop current montage if playing
    if (bIsPlayingMontage)
    {
        StopCurrentMontage(0.1f);
    }

    // Play the new montage
    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate * MontageData->PlayRate);
    if (MontageLength > 0.0f)
    {
        CurrentMontage = MontageName;
        bIsPlayingMontage = true;
        CurrentMontagePriority = MontageData->Priority;

        OnMontageStarted.Broadcast(MontageName);
        UE_LOG(LogTemp, Log, TEXT("MontageController: Playing montage '%s' with length %f"), *MontageName.ToString(), MontageLength);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("MontageController: Failed to play montage '%s'"), *MontageName.ToString());
    return false;
}

bool UAnim_MontageController::PlayMontageByType(EAnim_MontageType MontageType, float PlayRate, bool bForcePlay)
{
    FAnim_MontageData* MontageData = FindMontageDataByType(MontageType);
    if (!MontageData)
    {
        UE_LOG(LogTemp, Warning, TEXT("MontageController: No montage found for type %d"), (int32)MontageType);
        return false;
    }

    return PlayMontage(MontageData->MontageName, PlayRate, bForcePlay);
}

void UAnim_MontageController::StopCurrentMontage(float BlendOutTime)
{
    if (!AnimInstance || !bIsPlayingMontage)
    {
        return;
    }

    AnimInstance->Montage_Stop(BlendOutTime);
    UE_LOG(LogTemp, Log, TEXT("MontageController: Stopping current montage '%s'"), *CurrentMontage.ToString());
}

void UAnim_MontageController::StopAllMontages(float BlendOutTime)
{
    if (!AnimInstance)
    {
        return;
    }

    AnimInstance->StopAllMontages(BlendOutTime);
    bIsPlayingMontage = false;
    CurrentMontage = NAME_None;
    CurrentMontagePriority = -1;
    UE_LOG(LogTemp, Log, TEXT("MontageController: Stopping all montages"));
}

float UAnim_MontageController::GetMontagePosition() const
{
    if (!AnimInstance || !bIsPlayingMontage)
    {
        return 0.0f;
    }

    return AnimInstance->Montage_GetPosition(nullptr);
}

void UAnim_MontageController::SetMontagePosition(float Position)
{
    if (!AnimInstance || !bIsPlayingMontage)
    {
        return;
    }

    AnimInstance->Montage_SetPosition(nullptr, Position);
}

void UAnim_MontageController::AddMontageToDatabase(const FAnim_MontageData& MontageData)
{
    // Remove existing entry with same name
    RemoveMontageFromDatabase(MontageData.MontageName);
    
    // Add new entry
    MontageDatabase.Add(MontageData);
    UE_LOG(LogTemp, Log, TEXT("MontageController: Added montage '%s' to database"), *MontageData.MontageName.ToString());
}

void UAnim_MontageController::RemoveMontageFromDatabase(FName MontageName)
{
    int32 RemovedCount = MontageDatabase.RemoveAll([MontageName](const FAnim_MontageData& Data)
    {
        return Data.MontageName == MontageName;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("MontageController: Removed montage '%s' from database"), *MontageName.ToString());
    }
}

void UAnim_MontageController::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsPlayingMontage)
    {
        FName EndedMontageName = CurrentMontage;
        bIsPlayingMontage = false;
        CurrentMontage = NAME_None;
        CurrentMontagePriority = -1;

        OnMontageComplete.Broadcast(EndedMontageName, bInterrupted);
        UE_LOG(LogTemp, Log, TEXT("MontageController: Montage ended - '%s', Interrupted: %s"), 
               *EndedMontageName.ToString(), bInterrupted ? TEXT("true") : TEXT("false"));
    }
}

void UAnim_MontageController::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    // Handle blending out if needed
    UE_LOG(LogTemp, Log, TEXT("MontageController: Montage blending out"));
}

FAnim_MontageData* UAnim_MontageController::FindMontageData(FName MontageName)
{
    for (FAnim_MontageData& Data : MontageDatabase)
    {
        if (Data.MontageName == MontageName)
        {
            return &Data;
        }
    }
    return nullptr;
}

FAnim_MontageData* UAnim_MontageController::FindMontageDataByType(EAnim_MontageType MontageType)
{
    for (FAnim_MontageData& Data : MontageDatabase)
    {
        if (Data.MontageType == MontageType)
        {
            return &Data;
        }
    }
    return nullptr;
}

bool UAnim_MontageController::CanPlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay) const
{
    if (bForcePlay)
    {
        return true;
    }

    if (!bIsPlayingMontage)
    {
        return true;
    }

    // Check priority - higher priority can interrupt lower priority
    return MontageData.Priority >= CurrentMontagePriority;
}